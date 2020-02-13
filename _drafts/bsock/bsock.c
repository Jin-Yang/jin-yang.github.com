#include "bsock.h"

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>

#define MAX_PACKAGES   500

#ifndef BSOCK_DEF_CONNECT_TIMEOUT
#define BSOCK_DEF_CONNECT_TIMEOUT   10
#endif

#define MOD  "(sock) "
#define MODP "(sock) [%s:%s %s] "
#define MODA bs->ipaddr, bs->ipport, BS_STATE[bs->state]

const char *BS_STATE[BS_S_MAX] = {
	"inited",
	"listen",
	"sslshk",
	"coning",
	"conect",
	"shutdn",
	"failed",
	"closed",
};

static void buff_destroy(struct ev_buff *buff)
{
	if (buff == NULL)
		return;
	free(buff);
}

static struct ev_buff *buff_create(int size)
{
	struct ev_buff *buff;

	buff = malloc(sizeof(struct ev_buff) + size);
	if (buff == NULL)
		return NULL;
	buff->data = (char *)buff + sizeof(struct ev_buff);
	buff->start = buff->data;
	buff->tail = buff->data;
	buff->size = size;
	buff->end = buff->data + size;

	return buff;
}

int buff_drain(struct ev_buff *buff, int len)
{
	if (len >= buff->tail - buff->start) {
		buff->start = buff->data;
		buff->tail = buff->data;
		return 0;
	} else {
		buff->start += len;
		assert(buff->start <= buff->end);
		return buff_left(buff);
	}

	return 0;
}

int bsock_is_error_event(int event)
{
	if (event > BS_EV_MAX)
		return 1;
	return 0;
}

const char *bsock_get_event(int event)
{
	static const char *BSEV_STATE[BS_ERR_MAX] = {
		"",
		"timer",
		"SSL/TLS handshake finished",
		"connected",
		"read end of file",
		"finish processing",
		"",

		"create socket failed",
		"connect failed",
		"connect timeout",
		"write failed",
		"read failed",
		"not enough read buffer",
		"parse package failed",
	};

	if (event <= BS_EV_MIN || event >= BS_ERR_MAX || event == BS_EV_MAX)
		return "invalid event";

	return BSEV_STATE[event];
}

static void bsock_ssl_handshake_hook(EV_P_ struct ev_io *w, int revents)
{
	(void) revents;
	bsock_ssl_handshake(EV_A_ w->data);
}

static void bsock_limit_cb(EV_P_ ev_timer *w, int revents)
{
	(void) revents;
	struct ev_bsock *bs;

	bs = w->data;
	if (ev_is_active(&bs->rsock) == 0)
		ev_io_start(EV_A_ &bs->rsock);
}

static void bsock_timer_cb(EV_P_ ev_timer *w, int revents)
{
	(void) revents;
	struct ev_bsock *bs;

	bs = w->data;
	assert(ev_is_active(&bs->wtimer) == 0);
	log_trace(MODP "invoking timer callback.", MODA);

	switch (bs->state) {
	case BS_S_INIT:
	case BS_S_CONNECTED:
	case BS_S_CLOSED:
		if (bs->event)
			(*bs->event)(bs, BS_EV_TIMER, bs->arg);
		break;

	case BS_S_CONNECTING:
		bsock_close(EV_A_ bs, 0);
		bs->errcode = EEXIST;
		if (bs->event)
			(*bs->event)(bs, BS_ERR_CONN_TO, bs->arg);
		break;
	}
}

static void bsock_read_cb(EV_P_ ev_io *w, int revents)
{
	(void) revents;
	int rc, i, len;
	char *data;
	struct ev_bsock *bs;

	bs = w->data;
	log_trace(MODP "read callback for '%s:%s' on #%d.",
			MODA, bs->ipaddr, bs->ipport, bs->fd);

	/* need more buffer */
	rc = buff_left(bs->rbuf);
	if (rc <= 0) {
		log_trace(MODP "no more space for read socket.", MODA);
		bs->errcode = ENOSPC;
		if (bs->event)
			(*bs->event)(bs, BS_ERR_NOBUFF, bs->arg);
		bsock_close(EV_A_ bs, 0);
		return;
	}

	rc = read(w->fd, bs->rbuf->tail, rc);
	if (rc < 0) {
		if (errno == EINTR) {
			return;
		} else if (errno == EAGAIN) {
			usleep(10000);
			return;
		}
		bsock_close(EV_A_ bs, 0);
		bs->errcode = errno;
		if (bs->event)
			(*bs->event)(bs, BS_ERR_READ, bs->arg);
		return;
	} else if (rc == 0) {
		bsock_close(EV_A_ bs, 0);
		if (bs->event)
			(*bs->event)(bs, BS_EV_EOF, bs->arg);
		return;
	}
	bs->rbuf->tail += rc;

	len = buff_length(bs->rbuf);
	log_trace(MODP "read %d bytes, current %d bytes.", MODA, rc, len);
	assert(bs->package != NULL);

	/* in case of dead loop */
	data = bs->rbuf->start;
	for (i = 0; i < MAX_PACKAGES; i++) {
		rc = bs->package(data, len, bs->arg);
		if (rc == len) {
			buff_restart(bs->rbuf);
#if BS_SUPP_LIMIT == 1
			/* suppose this is the data instead of command etc. */
			if (bs->delay > 0) {
				ev_io_stop(EV_A_ &bs->rsock);
				ev_timer_set(&bs->ltimer, bs->delay, 0.0);
				ev_timer_start(EV_A_ &bs->ltimer);
			}
#endif

			return;
		} else if (rc == BS_RC_NEED_MORE) { /* need more data */
			assert(BS_RC_NEED_MORE == 0);
			if (data == bs->rbuf->start)
				return;
			len = bs->rbuf->tail - data;
			memmove(bs->rbuf->start, data, len);
			bs->rbuf->tail = bs->rbuf->start + len;
			return;
		} else if (rc < 0) { /* error */
			bsock_close(EV_A_ bs, 0);
			if (rc == BSOCK_PKG_FINISH)
				rc = BS_EV_FINISH;
			else
				rc = BS_ERR_PACKAGE;
			bs->errcode = EINVAL;
			if (bs->event)
				(*bs->event)(bs, rc, bs->arg);
			return;
		} else {
			data += rc;
			assert(data < bs->rbuf->tail);
			len -= rc;
			assert(len > 0);
		}
	}

	return;
}

static void bsock_write_cb(EV_P_ ev_io *w, int revents)
{
	(void) revents;
	struct ev_bsock *bs;
	int rc, len;

	bs = w->data;
	len = buff_length(bs->wbuf);
	log_debug(MODP "invoking write callback on %d, length %d bytes.",
			MODA, bs->fd, len);

	rc = write(bs->fd, bs->wbuf->start, len);
	if (rc < 0) {
		if (errno == EINTR)
			return;
		ev_io_start(EV_A_ &bs->wsock);
		bs->errcode = errno;
		if (bs->event)
			(*bs->event)(bs, BS_ERR_WRITE, bs->arg);
		bsock_close(EV_A_ bs, 0);
		return;
	} else if (rc > 0) {
		log_debug(MODP "write %d bytes done.", MODA, rc);
		if (buff_drain(bs->wbuf, rc) == 0) {
			log_trace(MODP "no more data to write.", MODA);
			ev_io_stop(EV_A_ &bs->wsock);
		}
		return;
	} else {
		ev_io_stop(EV_A_ &bs->wsock);
	}
}

static inline void bsock_connected(struct ev_bsock *bs)
{
	if (bs->tls->ctx != NULL) {
		if (bs->tls->ssl == NULL) {
			bs->tls->ssl = bsock_ssl_create(bs->tls->ctx, bs->fd, 0);
			if (bs->tls->ssl == NULL) {
				log_error(MOD "create SSL/TLS context failed.");
				bsock_destroy(EV_A_ bs, 0);
				return;
			}
		}
		assert(ev_is_active(&bs->rsock) == 0);
		assert(ev_is_active(&bs->wsock) == 0);
		ev_io_init(&bs->rsock, bsock_ssl_handshake_hook, bs->fd, EV_READ);
		bs->rsock.data = bs;
		ev_io_init(&bs->wsock, bsock_ssl_handshake_hook, bs->fd, EV_WRITE);
		bs->wsock.data = bs;
		bsock_ssl_handshake(EV_A_ bs);
		return;
	}

	bs->state = BS_S_CONNECTED;

	ev_init(&bs->wsock, bsock_write_cb);
	ev_io_set(&bs->wsock, bs->fd, EV_WRITE);
	bs->wsock.data = bs;

	ev_init(&bs->rsock, bsock_read_cb);
	ev_io_set(&bs->rsock, bs->fd, EV_READ);
	bs->rsock.data = bs;

#if BS_SUPP_LIMIT == 1
	if (bs->delay > 0) {
		ev_timer_set(&bs->ltimer, bs->delay, 0.0);
		ev_timer_start(EV_A_ &bs->ltimer);
	} else {
		ev_io_start(EV_A_ &bs->rsock);
	}
#else
	ev_io_start(EV_A_ &bs->rsock);
#endif

	if (bs->event)
		(*bs->event)(bs, BS_EV_CONNECT, bs->arg);
}

static void bsock_connect_hook(EV_P_ ev_io *w, int revents)
{
	(void) revents;
	int rc, error = 0;
	struct ev_bsock *bs;
	socklen_t errsz = sizeof(error);

	bs = w->data;
	log_trace(MODP "invoking connect callback.", MODA);

	assert(ev_is_active(&bs->wtimer));
	ev_timer_stop(EV_A_ &bs->wtimer);
	assert(ev_is_active(&bs->wsock));
	ev_io_stop(EV_A_ &bs->wsock);

	rc = getsockopt(bs->fd, SOL_SOCKET, SO_ERROR, (void *)&error, &errsz);
	if (rc < 0) {
		log_error(MODP "getsockopt failed, %d:%s.", MODA, errno, strerror(errno));
		bsock_close(EV_A_ bs, 0);
		bs->errcode = errno;
		if (bs->event)
			(*bs->event)(bs, BS_ERR_CONNECT, bs->arg);
		return;
	}

	if (error) {
		log_error(MODP "connect failed, %d:%s.", MODA, errno, strerror(error));
		bs->state = BS_S_FAILED;
		bsock_close(EV_A_ bs, 0);
		bs->errcode = error;
		if (bs->event)
			(*bs->event)(bs, BS_ERR_CONNECT, bs->arg);
		return;
	}

	bsock_connected(bs);
}

static void bsock_update_addr_port(struct ev_bsock *bs)
{
	struct sockaddr_in *ipv4;
	struct sockaddr_in6 *ipv6;

	if (bs->addr.family == AF_INET) {	  /* IPv4 */
		ipv4 = (struct sockaddr_in *)&bs->addr.ipv4;
		inet_ntop(AF_INET, &ipv4->sin_addr, bs->ipaddr, sizeof(bs->ipaddr));
		snprintf(bs->ipport, sizeof(bs->ipport), "%d", ntohs(ipv4->sin_port));
	} else if (bs->addr.family == AF_INET6) { /* IPv6 */
		ipv6 = (struct sockaddr_in6 *)&bs->addr.ipv6;
		inet_ntop(AF_INET, &ipv6->sin6_addr, bs->ipaddr, sizeof(bs->ipaddr));
		snprintf(bs->ipport, sizeof(bs->ipport), "%d", ntohs(ipv6->sin6_port));
	}
}

int bsock_set_socket(struct ev_bsock *bs, struct socketinfo *sock)
{
	if (bs == NULL || sock == NULL) {
		log_error(MODP "set socket failed, invalid arguments (%p, %p).",
			MODA, bs, sock);
		return -EINVAL;
	}

	if (bs->flag & BS_F_SVR) {
		if (bs->state == BS_S_LISTENING) {
			log_error(MODP "set socket failed, invalid state.", MODA);
			return -EINVAL;
		}
	} else {
		if (bs->state == BS_S_CONNECTING || bs->state == BS_S_CONNECTED) {
			log_error(MODP "set socket failed, invalid state.", MODA);
			return -EINVAL;
		}
	}

	if (sock->family != AF_INET) {
		log_error(MODP "only IPv4 support now, got %d.", MODA, sock->family);
		return -1;
	}
	memcpy(&bs->addr, sock, sizeof(struct socketinfo));
	bsock_update_addr_port(bs);

	bs->state = BS_S_INIT;
	log_debug(MODP "set sock address.", MODA);

	return 0;
}

int bsock_set_sock(struct ev_bsock *bs, struct addrinfo *addr)
{
	if (addr == NULL || bs == NULL) {
		log_error(MOD "set socket failed, invalid arguments (%p, %p).",
			bs, addr);
		return -1;
	}

	/* addr->ai_family != AF_INET6) */
	if (addr->ai_family != AF_INET) {
		log_error(MODP "only IPv4 support now, got %d.", MODA, addr->ai_family);
		return -1;
	}
	bs->addr.family = addr->ai_family;

	if (addr->ai_family == AF_INET) {	  /* IPv4 */
		struct sockaddr_in *ipv4;

		ipv4 = (struct sockaddr_in *)addr->ai_addr;
		inet_ntop(AF_INET, &ipv4->sin_addr, bs->ipaddr, sizeof(bs->ipaddr));
		snprintf(bs->ipport, sizeof(bs->ipport), "%d", ntohs(ipv4->sin_port));
		memcpy(&bs->addr.ipv4, addr->ai_addr, sizeof(struct sockaddr_in));
	} else if (addr->ai_family == AF_INET6) { /* IPv6 */
		struct sockaddr_in6 *ipv6;

		ipv6 = (struct sockaddr_in6 *)addr->ai_addr;
		inet_ntop(AF_INET, &ipv6->sin6_addr, bs->ipaddr, sizeof(bs->ipaddr));
		snprintf(bs->ipport, sizeof(bs->ipport), "%d", ntohs(ipv6->sin6_port));
		memcpy(&bs->addr.ipv6, addr->ai_addr, sizeof(struct sockaddr_in6));
	}

	log_debug(MOD "set sock address to '%s:%s'.", bs->ipaddr, bs->ipport);

	return 0;
}

static void bsock_accept_hook(EV_P_ struct ev_io *w, int revents)
{
	(void) revents;
	int fd;
	socklen_t socklen;
	struct ev_bsock *bs, *cli;

	bs = w->data;
	cli = bsock_create(4096, 4096, BS_F_SSL);
	if (cli == NULL) {
		log_error(MODP "accept failed, out of memory.", MODA);
		return;
	}

	socklen = sizeof(cli->addr.ipv4);
	fd = accept(w->fd, (struct sockaddr *)&cli->addr.ipv4, &socklen);
	if (fd < 0) {
		log_error(MODP "accept failed, %d:%s.", MODA, errno, strerror(errno));
		bsock_destroy(EV_A_ cli, 0);
		return;
	}
	cli->fd = fd;
	cli->addr.family = AF_INET;
	bsock_update_addr_port(cli);

	cli->arg = bs->arg;
	cli->event = bs->event;

	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
		log_error(MODP "set socket NONBLOCK failed, %d:%s.", MODA, errno, strerror(errno));
		bsock_destroy(EV_A_ cli, 0);
		return;
	}

	cli->tls->ctx = bs->tls->ctx;
	cli->tls->ssl = bsock_ssl_create(cli->tls->ctx, fd, BS_F_SVR);
	if (cli->tls->ssl == NULL) {
		log_error(MOD "create SSL failed.");
		bsock_destroy(EV_A_ cli, 0);
		return;
	}
	log_info(MODP "client '%s:%s' connected, fd %d.", MODA, cli->ipaddr, cli->ipport, cli->fd);

	//SSL_CTX_set_num_tickets(sslctx, 0);
	SSL_set_num_tickets(cli->tls->ssl, 2);

	ev_io_init(&cli->rsock, bsock_ssl_handshake_hook, cli->fd, EV_READ);
	cli->rsock.data = cli;
	ev_io_init(&cli->wsock, bsock_ssl_handshake_hook, cli->fd, EV_WRITE);
	cli->wsock.data = cli;
	//ctx->ssl.hook = ssock_server_end_ssl_handshake;
	cli->state = BS_S_SSL_HANDSHAKE;

	bsock_ssl_handshake(EV_A_ cli);
}

int bsock_listen(struct ev_bsock *bs)
{
	int fd, value;

	assert((bs->flag & BS_F_SVR));
	if (bs->state == BS_S_LISTENING) {
		log_warning(MODP "socket is listening, ignore this.", MODA);
		return 0;
	}

	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		log_error(MOD "create socket failed, %d:%s.", errno, strerror(errno));
		bs->errcode = errno;
		if (bs->event)
			(*bs->event)(bs, BS_ERR_SOCKET, bs->arg);
		return -1;
	}

	value = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&value, sizeof(value)) < 0) {
		log_error(MOD "set socket option REUSEADDR failed, %d:%s.", errno, strerror(errno));
		bs->errcode = errno;
		close(fd);
		if (bs->event)
			(*bs->event)(bs, BS_ERR_SOCKET, bs->arg);
		return -1;
	}

	if (bind(fd, (struct sockaddr *)&bs->addr.ipv4, sizeof(bs->addr.ipv4)) < 0) {
		log_error(MOD "bind socket failed, %d:%s.", errno, strerror(errno));
		bs->errcode = errno;
		close(fd);
		if (bs->event)
			(*bs->event)(bs, BS_ERR_SOCKET, bs->arg);
		return -1;
	}

	if (listen(fd, 128) < 0) {
		log_error(MOD "listen socket failed, %d:%s.", errno, strerror(errno));
		bs->errcode = errno;
		close(fd);
		if (bs->event)
			(*bs->event)(bs, BS_ERR_SOCKET, bs->arg);
		return -1;
	}
	bs->fd = fd;
	bs->state = BS_S_LISTENING;
	log_info(MODP "listening now.", MODA);
	assert(ev_is_active(&bs->wsock) == 0);

	ev_io_init(&bs->wsock, bsock_accept_hook, fd, EV_READ | EV_WRITE);
	bs->wsock.data = bs;
	ev_io_start(EV_A_ &bs->wsock);

	return 0;
}

int bsock_connect(struct ev_bsock *bs)
{
	int fd, rc;

	assert((bs->flag & BS_F_SVR) == 0);
	if (bs->state == BS_S_CONNECTED || bs->state == BS_S_CONNECTING) {
		log_warning(MODP "connect is in processing, ignore this.", MODA);
		return 0;
	}
	log_debug(MODP "try to connect, timeout %ds.", MODA, bs->connect_timeout);

	fd = socket(bs->addr.family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd == -1) {
		log_error(MODP "create socket failed, %d:%s.", MODA, errno, strerror(errno));
		bs->errcode = errno;
		if (bs->event)
			(*bs->event)(bs, BS_ERR_SOCKET, bs->arg);
		return -1;
	}
	bs->fd = fd;

	rc = connect(fd, (struct sockaddr *)&bs->addr.ipv4, sizeof(struct sockaddr));
	if (rc == -1 && errno != EINPROGRESS) {
		log_error(MODP "connect failed, %d:%s.", MODA, errno, strerror(errno));
		close(fd);
		bs->errcode = errno;
		if (bs->event)
			(*bs->event)(bs, BS_ERR_CONNECT, bs->arg);
		return -1;
	} else if (rc == 0) {
		log_info(MODP "connect success immediately.", MODA);
		bsock_connected(bs);
		return fd;
	}
	bs->state = BS_S_CONNECTING;

	assert(ev_is_active(&bs->wsock) == 0);
	ev_init(&bs->wsock, bsock_connect_hook);
	ev_io_set(&bs->wsock, bs->fd, EV_WRITE);
	bs->wsock.data = bs;
	ev_io_start(EV_A_ &bs->wsock);

	assert(ev_is_active(&bs->wtimer) == 0);
	ev_init(&bs->wtimer, bsock_timer_cb);
	ev_timer_set(&bs->wtimer, bs->connect_timeout, 0.);
	bs->wtimer.data = bs;
	ev_timer_start(EV_A_ &bs->wtimer);

	return bs->fd;
}

#define MIN(a, b) if ((a) > (b) ?

int bsock_write(struct ev_bsock *bs, const char *data, int len)
{
	int left, written;
	struct ev_buff *buff;

	if (bs->state != BS_S_CONNECTED)
		return -1;
	buff = bs->wbuf;
	left = buff_left(buff);
	log_trace(MOD "writing %d bytes starting at %p, left %d.",
			len, data, left);
	if (left == 0)
		return 0;

	written = left > len ? len : left;
	memcpy(buff->tail, data, written);
	buff->tail += written;

	if (ev_is_active(&bs->wsock) == 0) {
		ev_init(&bs->wsock, bsock_write_cb);
		ev_io_set(&bs->wsock, bs->fd, EV_WRITE);
		bs->wsock.data = bs;
		ev_io_start(EV_A_ &bs->wsock);
	}

	return written;
}

void bsock_reset(EV_P_ struct ev_bsock *bs)
{
	if (bs == NULL)
		return;
	log_trace(MODP "reset.", MODA);

	ev_io_stop(EV_A_ &bs->wsock);
	ev_io_stop(EV_A_ &bs->rsock);
	ev_timer_stop(EV_A_ &bs->wtimer);
	ev_timer_stop(EV_A_ &bs->rtimer);
#if BS_SUPP_LIMIT == 1
	ev_timer_stop(EV_A_ &bs->ltimer);
#endif

	if (bs->wbuf != NULL)
		buff_restart(bs->wbuf);
	if (bs->rbuf != NULL)
		buff_restart(bs->rbuf);
}

void bsock_shutdown(EV_P_ struct ev_bsock *bs, int flag)
{
	if (bs == NULL)
		return;
	if (bs->state == BS_S_SHUTDOWN || bs->state == BS_S_CLOSED)
		return;
	log_trace(MODP "shutdown %s session.", MODA, flag & BS_F_KEP ? "keep" : "clean");

	bsock_reset(EV_A_ bs);
	bs->state = BS_S_SHUTDOWN;
	if (bs->tls == NULL || bs->tls->ssl == NULL)
		return;

	if (flag & BS_F_KEP)
		SSL_set_shutdown(bs->tls->ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
	else
		bsock_tls_shutdown(bs->tls->ssl);
}

/* close all socket, and stop timer, don't destroy the structure. */
void bsock_close(EV_P_ struct ev_bsock *bs, int flag)
{
	if (bs == NULL)
		return;
	if (bs->state == BS_S_CLOSED || bs->state == BS_S_INIT)
		return;
	log_trace(MODP "close on fd #%d.", MODA, bs->fd);

	bsock_shutdown(EV_A_ bs, flag);
	if (bs->fd >= 0)
		close(bs->fd);
	bs->fd = -1;
	bs->state = BS_S_CLOSED;
}

static void bsock_do_destroy(struct ev_bsock *bs)
{
	if (bs->tls != NULL) {
		if (bs->tls->ssl != NULL)
			SSL_free(bs->tls->ssl);
		if ((bs->flag & BS_F_CTX) && bs->tls->ctx != NULL)
			SSL_CTX_free(bs->tls->ctx);
	}

	if (bs->wbuf)
		buff_destroy(bs->wbuf);
	if (bs->rbuf)
		buff_destroy(bs->rbuf);
	free(bs);
}

void bsock_destroy(EV_P_ struct ev_bsock *bs, int flag)
{
	if (bs == NULL)
		return;
	log_trace(MODP "destroy.", MODA);

	bsock_close(EV_A_ bs, flag);
	bsock_do_destroy(bs);
}

struct ev_bsock *bsock_create(int wbsize, int rbsize, int flag)
{
	struct ev_bsock *bs;
	int len = sizeof(struct ev_bsock);

	if (flag & BS_F_SSL)
		len += sizeof(struct ev_bstls);
	bs = calloc(1, len);
	if (bs == NULL)
		return NULL;
	bs->fd = -1;
	bs->flag = flag;
	bs->state = BS_S_INIT;
	bs->connect_timeout = BSOCK_DEF_CONNECT_TIMEOUT;
	if (flag & BS_F_SSL) {
		bs->tls = (struct ev_bstls *)((char *)bs + sizeof(*bs));
		if (flag & BS_F_CTX) {
			bs->tls->ctx = bsock_ssl_create_context(flag);
			if (bs->tls->ctx == NULL) {
				bsock_do_destroy(bs);
				return NULL;
			}
		}
	}

	if (wbsize > 0) {
		bs->wbuf = buff_create(wbsize);
		if (bs->wbuf == NULL) {
			bsock_do_destroy(bs);
			return NULL;
		}
	}

	if (rbsize > 0) {
		bs->rbuf = buff_create(rbsize);
		if (bs->rbuf == NULL) {
			bsock_do_destroy(bs);
			return NULL;
		}
	}

#if BS_SUPP_LIMIT == 1
	ev_init(&bs->ltimer, bsock_limit_cb);
	bs->ltimer.data = bs;
#endif

	return bs;
}
