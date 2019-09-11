
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define LOG_TIME  1

#include "log_simple.c"

#include "libev/ev.h"

#define MOD "(sock) "

#define THD_NUMS       1
#define CLI_CONN_TO    10   /* connect timeout */

struct proto_hdr {
	uint8_t magic[4];
	uint16_t type;
	uint32_t length;
} __attribute__((packed));
#define SVR_P_HDR_MIN   10
#define SVR_P_MAGIC_0   'A'
#define SVR_P_MAGIC_1   'G'
#define SVR_P_MAGIC_2   'T'
#define SVR_P_MAGIC_3   '1'

enum {
	S_INIT,
	S_START,
	S_ESTB,
	S_FAILED,
	S_MAX
};
const char *CLIS[S_MAX] = {
	"init",
	"start",
	"establish",
	"failed",
};

#define CLI_RERND_CNT 20
struct sockinfo {
	int naddr, max, next, count;
	struct sockaddr *addrs;
};
struct clictx {
	char *oaddr; /* orignal address */
	struct sockinfo socks;

	int state;
	int retry_secs;

	struct ev_loop *loop;

	struct ev_io wsock;
	struct ev_timer wretry;
	struct ev_timer wtimeout;
};

static ssize_t sock_write_fully(int fd, const char *buff, ssize_t nbytes)
{
	int rc;
	ssize_t nwritten;

	for (nwritten = 0; nwritten < nbytes;) {
		rc = write(fd, buff + nwritten, nbytes - nwritten);
		if (rc < 0) {
			if (errno == EINTR) {
				continue;
			} else if (errno == EAGAIN) {
				usleep(10000);
				continue;
			}

			log_error(MOD "write to #%d failed, rc %d, %d:%s.",
					fd, rc, errno, strerror(errno));
			return -1;
		} else if (rc == 0) {
			break;
		}
		nwritten += rc;
	}

	return nwritten;
}
/* NOTE: this will block while parse the address. */
static int sock_do_parse_address(struct sockinfo *socks, char *addr)
{
	void *tmp;
	int rc, nsize;
	char *port = NULL;
	struct sockaddr_in *ipv4;
	char ipaddr[INET_ADDRSTRLEN];
	struct addrinfo hints, *res, *curr;

	port = strchr(addr, ':');
	if (port != NULL) {
		*port = 0;
		port++;
	}

	memset_s(&hints, sizeof(hints), 0, sizeof(hints));
	hints.ai_family = AF_INET;     /* AF_UNSPEC AF_INET(IPv4) AF_INET6(IPv6) */
	hints.ai_socktype = SOCK_STREAM; /* TCP stream sockets */

	rc = getaddrinfo(addr, port, &hints, &res);
	if (rc) {
		log_error(MOD "getaddrinfo for '%s' failed, %d:%s.",
				addr, rc, gai_strerror(rc));
		return -1;
	}

	for (curr = res; curr != NULL; curr = curr->ai_next) {
		if (curr->ai_family != AF_INET) {
			log_error(MOD "got invalid family %d for '%s'.",
					curr->ai_family, addr);
			continue;
		}

		if (socks->naddr + 1 > socks->max) {
			assert(socks->max == 0);
			assert(socks->naddr == 0);
			if (socks->max == 0)
				nsize = 8; /* min */
			else
				nsize = socks->max * 2;

			tmp = realloc(socks->addrs, nsize * sizeof(*(socks->addrs))); /* CodeMars */
			if (tmp == NULL) {
				log_error(MOD "parse address '%s' failed, out of memory.", addr);
				freeaddrinfo(res);
				return -1;
			}
			socks->max = nsize;
			socks->addrs = (struct sockaddr *)tmp;
		}
		ipv4 = (struct sockaddr_in *)curr->ai_addr;
		memcpy_s(&socks->addrs[socks->naddr++], sizeof(struct sockaddr),
				ipv4, sizeof(struct sockaddr));
		inet_ntop(ipv4->sin_family, &ipv4->sin_addr, ipaddr, sizeof(ipaddr));
		log_info(MOD "got IP '%s:%d', current %d items.", ipaddr,
				ntohs(ipv4->sin_port), socks->naddr);
	}
	freeaddrinfo(res);

	return 0;
}
/* addrs: 'svr.foobar.com:8090,127.1:10086' */
static int sock_parse_address(struct clictx *ctx, const char *addrstr)
{
	int len;
	struct sockinfo socks;
	char *tmp, *ptr, *saveptr, *addr;

	assert(ctx && addrstr);

	if (addrstr[0] == 0)
		return -1;
	memset(&socks, 0, sizeof(socks));
	len = strlen(addrstr);

	tmp = (char *)malloc((len + 1) * 2);
	if (tmp == NULL) {
		log_error(MOD "parse address failed, out of memory.");
		return -1;
	}
	ptr = tmp + len + 1;
	strncpy(tmp, addrstr, len);
	strncpy(ptr, addrstr, len);

	saveptr = NULL;
	while ((addr = strtok_s(ptr, ",", &saveptr)) != NULL) {
		ptr = NULL;
		log_info(MOD "get address '%s'.", addr);

		if (sock_do_parse_address(&socks, addr) < 0) {
			if (socks.addrs)
				free(socks.addrs);
			free(tmp);
			log_info(MOD "address '%s' parse failed, use old config.", addrstr);
			return -1;
		}
	}

	log_info(MOD "update address info from %d items to %d items.",
			ctx->socks.naddr, socks.naddr);
	if (ctx->socks.addrs)
		free(ctx->socks.addrs);
	memcpy(&ctx->socks, &socks, sizeof(socks));

	if (ctx->oaddr)
		free(ctx->oaddr);
	ctx->oaddr = tmp;

	return 0;
}
static struct sockaddr *sock_choose_server(struct sockinfo *sock)
{
	if (sock->naddr == 1) /* fast path */
		return &sock->addrs[0];

	if (sock->count++ > CLI_RERND_CNT) {
		sock->count = 0;
		sock->next = -1;
	}

	if (sock->next < 0) {
		sock->next = rand() % sock->naddr;
		log_debug(MOD "choose a random server, start with %d, current %d.",
				sock->next, sock->naddr);
	}

	if (sock->next + 1 >= sock->naddr)
		sock->next = 0;
	else
		sock->next++;
	log_debug(MOD "choose %dth server.", sock->next);
	return &sock->addrs[sock->next];
}

static void sock_context_reset(EV_P_ struct clictx *ctx)
{
	assert(ctx);
	assert(ev_is_active(&ctx->wretry) == 0);

	if (ev_is_active(&ctx->wsock))
		ev_io_stop(EV_A_ &ctx->wsock);
	if (ctx->wsock.fd > 0) {
		//close(ctx->wsock.fd);
		shutdown(ctx->wsock.fd, SHUT_RDWR);
		ctx->wsock.fd = -1;
	}
}

void sock_retry_later(EV_P_ struct clictx *ctx)
{
	if (ev_is_active(&ctx->wretry)) {
		log_info(MOD "retry is in progress, ignore this now.");
		return;
	}
	sock_context_reset(EV_A_ ctx);
	assert(ctx->wsock.fd < 0);

	ctx->state = S_FAILED;
	ev_timer_set(&ctx->wretry, ctx->retry_secs, 0.);
	ev_timer_start(EV_A_ &ctx->wretry);
	log_info(MOD "retry in %d seconds later.", ctx->retry_secs);
}
void sock_read_hook(EV_P_ struct ev_io *w, int revents)
{
	(void) revents;
	int rc;
	char buff[256];
	struct clictx *ctx;

	ctx = (struct clictx *)w->data;

	rc = read(w->fd, buff, sizeof(buff));
	if (rc == 0) {
		log_error(MOD "read 0 byte from socket, peer maybe close.");
		sock_retry_later(EV_A_ ctx);
		return;
	} else if (rc < 0) {
		if (errno == EINTR) {
			return;
		} else if (errno == EAGAIN) {
			usleep(10000);
			return;
		}

		log_error(MOD "read from socket failed, %d:%s.", errno, strerror(errno));
		sock_retry_later(EV_A_ ctx);
		return;
	}
	log_debug(MOD "receive %d bytes from socket: %.*s", rc, rc, buff);
}

static void sock_handle_connected(EV_P_ struct clictx *ctx, int sockfd)
{
	assert(ev_is_active(&ctx->wsock) == 0);

	ctx->state = S_ESTB;
	log_info(MOD "connected success, sockfd %d.", sockfd);

	ev_io_init(&ctx->wsock, sock_read_hook, sockfd, EV_READ);
	ctx->wsock.data = ctx;
	ev_io_start(EV_A_ &ctx->wsock);
}
void sock_connect_hook(EV_P_ struct ev_io *w, int revents)
{
	(void) revents;
	int rc, sockfd, serror;
	socklen_t err_len;
	struct clictx *ctx;
	ctx = (struct clictx *)w->data;

	if (ev_is_active(&ctx->wsock))
		ev_io_stop(EV_A_ &ctx->wsock);
	if (ev_is_active(&ctx->wtimeout))
		ev_timer_stop(EV_A_ &ctx->wtimeout);
	sockfd = w->fd;
	log_info(MOD "connected async, current state '%s'.", CLIS[ctx->state]);

	serror = 0;
	err_len = sizeof(serror);
	rc = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void *)&serror, &err_len);
	if (rc < 0) {
		log_error(MOD "get socket option failed, %d:%s.",
				errno, strerror(errno));
		sock_retry_later(EV_A_ ctx);
		return;
	}

	if (serror != 0) {
		log_error(MOD "connect failed, %d:%s.", serror, strerror(serror));
		sock_retry_later(EV_A_ ctx);
		return;
	}

	sock_handle_connected(EV_A_ ctx, sockfd);
}
/* try to connect to a server, if failed start a retry timer. */
static int sock_connect_to(EV_P_ struct clictx *ctx, struct sockaddr_in *addr)
{
	int sockfd, rc;
	struct linger linger;
	char ipaddr[INET_ADDRSTRLEN];

	inet_ntop(addr->sin_family, &addr->sin_addr, ipaddr, sizeof(ipaddr));
	log_info(MOD "connect to '%s:%d'.", ipaddr, ntohs(addr->sin_port));

	sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (sockfd < 0) {
		log_error(MOD "create socket failed, %d:%s.", errno, strerror(errno));
		return -1;
	}
	assert(ev_is_active(&ctx->wsock) == 0);

	linger.l_onoff = 1;
	linger.l_linger = 0;
	rc = setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
	if (rc < 0) {
		log_error(MOD "set socket LINGER option failed, %d:%s.",
				errno, strerror(errno));
		close(sockfd);
		return -1;
	}

	rc = connect(sockfd, (struct sockaddr *)addr, sizeof(*addr));
	if (rc < 0) {
		if (errno == EINPROGRESS) {
			ctx->state = S_START;
			ev_timer_set(&ctx->wtimeout, CLI_CONN_TO, 0.);
			ev_timer_start(EV_A_ &ctx->wtimeout);

			ev_io_init(&ctx->wsock, sock_connect_hook, sockfd, EV_WRITE);
			ctx->wsock.data = ctx;
			ev_io_start(EV_A_ &ctx->wsock);

			log_info(MOD "connect to server async, timeout %ds.", CLI_CONN_TO);
			return 0;
		}

		log_error(MOD "connect to server failed, %d:%s.", errno, strerror(errno));
		close(sockfd);
		return -1;
	}

	sock_handle_connected(EV_A_ ctx, sockfd);

	return 0;
}
void sock_retry_hook(EV_P_ ev_timer *w, int revents)
{
	(void) revents;
	struct clictx *ctx;
	struct sockaddr *svr;

	ctx = (struct clictx *)w->data;
	assert(ev_is_active(&ctx->wsock) == 0);
	assert(ev_is_active(&ctx->wtimeout) == 0);
	assert(ctx->state == S_INIT || ctx->state == S_FAILED);

	log_debug(MOD "retry timer timeout, try to connect to server.");
	if (ev_is_active(&ctx->wtimeout))
		ev_timer_stop(EV_A_ &ctx->wtimeout);

	svr = sock_choose_server(&ctx->socks);
	assert(svr != NULL);

	if (sock_connect_to(EV_A_ ctx, (struct sockaddr_in *)svr) < 0) {
		sock_retry_later(EV_A_ ctx);
		return;
	}
}

void sock_timeout_hook(EV_P_ ev_timer *w, int revents)
{
	(void) revents;
	struct clictx *ctx;

	ctx = (struct clictx *)w->data;
	log_error(MOD "timeout, current state '%s'.", CLIS[ctx->state]);
	sock_retry_later(EV_A_ ctx);
}


void sock_thd_retry_hook(EV_P_ ev_timer *w, int revents)
{
	(void) revents;
	ssize_t rc;
	char buff[1024];
	struct clictx *ctx;
	struct proto_hdr hdr;
	unsigned int cnt = 0; // UINT_MAX 4294967295U

	hdr.type = 0;
	hdr.magic[0] = SVR_P_MAGIC_0;
	hdr.magic[1] = SVR_P_MAGIC_1;
	hdr.magic[2] = SVR_P_MAGIC_2;
	hdr.magic[3] = SVR_P_MAGIC_3;
	hdr.length = 20;

	memcpy(buff, &hdr, SVR_P_HDR_MIN);

	log_info(MOD "start to send data.");
	ctx = (struct clictx *)w->data;
	while (1) {
		cnt++;
		snprintf(buff + SVR_P_HDR_MIN, 1024, "%010d%010d", cnt, cnt);

		rc = sock_write_fully(ctx->wsock.fd, buff, hdr.length + SVR_P_HDR_MIN);
		if (rc < 0) {
			log_info(MOD "current count %d.", cnt);
			ev_timer_set(w, 3., 0.);
			ev_timer_start(EV_A_ w);
			return;
		}
	}
}

void *thd_worker(void *arg)
{
	struct clictx *ctx;
	struct ev_timer wtimer;
	struct ev_loop *loop = ev_loop_new(0);

	if (loop == NULL) {
		log_error(MOD "create libev loop failed, out of memory.");
		return NULL;
	}

	ctx = (struct clictx *)arg;
	log_info(MOD "worker thread started.");

	ev_init(&wtimer, sock_thd_retry_hook);
	wtimer.data = ctx;
	ev_timer_set(&wtimer, 5., 0.);
	ev_timer_start(EV_A_ &wtimer);

	ev_run(EV_A_ 0);

	return NULL;
}

int main(void)
{
	int i, rc;
	pthread_t thds[THD_NUMS];
	struct clictx ctxs;
	struct ev_loop *loop = ev_default_loop(0);

	signal(SIGPIPE, SIG_IGN);
	memset(&ctxs, 0, sizeof(ctxs));
	if (sock_parse_address(&ctxs, "192.168.170.220:10087") < 0) {
		log_error(MOD "parse address failed.");
		return -1;
	}
	ctxs.wsock.fd = -1;

	ctxs.retry_secs = 5;
	ev_init(&ctxs.wretry, sock_retry_hook);
	ctxs.wretry.data = &ctxs;

	ev_init(&ctxs.wtimeout, sock_timeout_hook);
	ctxs.wtimeout.data = &ctxs;

	ev_timer_set(&ctxs.wretry, 3., 0.);
	ev_timer_start(EV_A_ &ctxs.wretry);

	for (i = 0; i < THD_NUMS; i++) {
		rc = pthread_create(&thds[i], NULL, thd_worker, &ctxs);
		if (rc != 0) {
			log_error(MOD "create thread failed, %d:%s.",
					rc, strerror(rc));
			exit(1);
		}
	}

	rc = ev_run(EV_A_ 0);
	log_info(MOD "still remain %d watchers.", rc);

	for (i = 0; i < THD_NUMS; i++)
		pthread_join(thds[i], NULL);

	return 0;
}
