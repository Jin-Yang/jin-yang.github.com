#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#define LOG_TIME  1

#include "log_simple.c"

#include "libev/ev.h"

#define MOD "(sock) "

#define SVR_PORT       10087
#define SVR_IDLE_TO    3.   /* idle timeout */
#define SVR_BUFF_SIZE  4096

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

struct svr_ctx {
	int len;
	char *buff, *tail, *end;

	struct sockaddr_in addr;

	struct ev_io wsock;
	struct ev_timer wtimeout;
};

static void sock_context_destroy(EV_P_ struct svr_ctx *ctx)
{
	if (ctx == NULL)
		return;

	if (ev_is_active(&ctx->wsock)) {
		assert(ctx->wsock.fd > 0);
		ev_io_stop(EV_A_ &ctx->wsock);
		close(ctx->wsock.fd);
		ctx->wsock.fd = -1;
	}

	if (ev_is_active(&ctx->wtimeout))
		ev_timer_stop(EV_A_ &ctx->wtimeout);

	if (ctx->buff != NULL)
		free(ctx->buff);
	free(ctx);
}
static int sock_valid_check(struct proto_hdr *hdr)
{
	if (hdr->magic[0] != SVR_P_MAGIC_0 || hdr->magic[1] != SVR_P_MAGIC_1 ||
			hdr->magic[2] != SVR_P_MAGIC_2 || hdr->magic[3] != SVR_P_MAGIC_3)
		return -1;

	if (hdr->length > SVR_BUFF_SIZE - SVR_P_HDR_MIN - 1) {
		log_error(MOD "package size overflow, got %d > %d.",
				hdr->length, SVR_BUFF_SIZE - SVR_P_HDR_MIN - 1);
		return -1;
	}

	return 0;
}

static int sock_handle_package(char *buff, int len)
{
	struct proto_hdr hdr;

	if (len < SVR_P_HDR_MIN)
		return 0;
	memcpy_s(&hdr, SVR_P_HDR_MIN, buff, SVR_P_HDR_MIN);
	if (sock_valid_check(&hdr) < 0) {
		log_error(MOD "invalid header: 0x%02x%02x%02x%02x%02x%02x%02x.",
				(unsigned char)buff[0],
				(unsigned char)buff[1],
				(unsigned char)buff[2],
				(unsigned char)buff[3],
				(unsigned char)buff[4],
				(unsigned char)buff[5],
				(unsigned char)buff[6]);
		return -1;
	}

	if (len < SVR_P_HDR_MIN + (int)hdr.length) {
		log_info(MOD "need more data, got length(%d).", hdr.length);
		return 0;
	}

	log_info(MOD "got request type(0x%04x) length(%d).", hdr.type, hdr.length);
	log_info(MOD "got data: %.*s", hdr.length, buff + SVR_P_HDR_MIN);

	return hdr.length + SVR_P_HDR_MIN;
}
static void sock_read_hook(EV_P_ struct ev_io *w, int revents)
{
	char *buff;
	int rc, len, left;
	struct svr_ctx *ctx;

	if (EV_ERROR & (unsigned int)revents) {
		log_error(MOD "invoking client failed, got invalid event.");
		return;
	}

	ctx = (struct svr_ctx *)w->data;
	left = ctx->end - ctx->tail;
	log_info(MOD "invoking client hook, got event 0x%x, left %d.", revents, left);
	assert(left > 0);

	rc = recv(w->fd, ctx->tail, left, 0);
	if (rc < 0) {
		log_error(MOD "receive from #%d failed, %d:%s.", w->fd,
				errno, strerror(errno));
		sock_context_destroy(EV_A_ ctx);
		return;
	} else if (rc == 0) {
		log_error(MOD "peer #%d maybe closed.", w->fd);
		sock_context_destroy(EV_A_ ctx);
		return;
	}

	ctx->tail += rc;
	buff = ctx->buff;
	len = ctx->tail - ctx->buff; /* current received buffer length */
	log_info(MOD "receive %d bytes from socket, total %d.", rc, len);

	while (1) {
		rc = sock_handle_package(buff, len);
		if (rc == 0) { /* need more data */
			len = ctx->tail - buff;
			memmove(ctx->buff, buff, len);
			ctx->tail = ctx->buff + len;
			return;
		} else if (rc < 0) {
			sock_context_destroy(EV_A_ ctx);
			return;
		} else if (rc == len) {
			ctx->tail = ctx->buff;
			return;
		}

		assert(len > rc);
		len -= rc;
		buff += rc;
	}

	if (ctx->tail >= ctx->end) {
		log_error(MOD "receive buffer full.");
		sock_context_destroy(EV_A_ ctx);
		return;
	}
}


static void sock_client_timeout_hook(EV_P_ struct ev_timer *w, int revents)
{
	(void) revents;
	struct svr_ctx *ctx;

	ctx = (struct svr_ctx *)w->data;
	log_error(MOD "socket #%d timeout.", ctx->wsock.fd);
	sock_context_destroy(EV_A_ ctx);
}

static void sock_accept_hook(EV_P_ struct ev_io *w, int revents)
{
	int sockfd;
	struct svr_ctx *ctx;
	socklen_t socklen = sizeof(ctx->addr);

	if (EV_ERROR & revents) {
		log_error(MOD "invoking accept failed, got invalid event.");
		return;
	}
	log_info(MOD "invoking accept hook, got event 0x%x.", revents);

	ctx = (struct svr_ctx *)calloc(1, sizeof(*ctx));
	if (ctx == NULL) {
		log_error(MOD "accept failed, out of memory.");
		return;
	}
	ctx->wsock.fd = -1;
	ctx->len = SVR_BUFF_SIZE;

	ctx->buff = (char *)malloc(ctx->len);
	if (ctx->buff == NULL) {
		log_error(MOD "accept failed, out of memory.");
		free(ctx);
		return;
	}
	ctx->tail = ctx->buff;
	ctx->end = ctx->buff + ctx->len;

	sockfd = accept(w->fd, (struct sockaddr *)&ctx->addr, &socklen);
	if (sockfd < 0) {
		log_error(MOD "accept failed, %d:%s.", errno, strerror(errno));
		sock_context_destroy(EV_A_ ctx);
		return;
	}
	log_info(MOD "client '%s:%d' connected, fd %d.",
			inet_ntoa(ctx->addr.sin_addr), ntohs(ctx->addr.sin_port), sockfd);

	ev_io_init(&ctx->wsock, sock_read_hook, sockfd, EV_READ);
	ctx->wsock.data = ctx;
	ev_io_start(loop, &ctx->wsock);

	ev_timer_init(&ctx->wtimeout, sock_client_timeout_hook, SVR_IDLE_TO, SVR_IDLE_TO);
	ctx->wtimeout.data = ctx;
	ev_timer_start(EV_A_ &ctx->wtimeout);
}
int main(void)
{
	int sockfd, opt;
	struct ev_io wsvr;
	struct sockaddr_in addr;
	struct ev_loop *loop = ev_default_loop(0);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		log_error(MOD "create socket failed, %d:%s.", errno, strerror(errno));
		return -1;
	}

	opt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		log_error(MOD "set socket REUSEADDR failed, %d:%s.", errno,
				strerror(errno));
		close(sockfd);
		return -1;
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SVR_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		log_error(MOD "bind socket failed, %d:%s.", errno, strerror(errno));
		close(sockfd);
		return -1;
	}

	if (listen(sockfd, SOMAXCONN) < 0) {
		log_error(MOD "listen socket failed, %d:%s.", errno, strerror(errno));
		close(sockfd);
		return -1;
	}

	//ev_io_init(&wsvr, sock_accept_hook, sockfd, EV_READ);
	ev_io_init(&wsvr, sock_accept_hook, sockfd, EV_READ | EV_WRITE);
	ev_io_start(EV_A_ &wsvr);

	log_info(MOD "start to listen ':%d'.", SVR_PORT);

	/* now wait for events to arrive. */
	ev_run(EV_A_ 0);

	log_info(MOD "leave now.");

	return 0;
}
