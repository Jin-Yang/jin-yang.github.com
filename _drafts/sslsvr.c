#include <assert.h>

#include "libev/ev.h"
#include "../../socket/common.h"
#include "../../socket/bsock/bsock.h"

#define MOD "(main) "

enum {
	APP_INIT,
	APP_READ,
	APP_WRITE,
} state;

static void server_app_process_hook(EV_P_ struct ev_io *w, int revents)
{
	(void)revents;
	int rc;
	struct ev_bsock *bs;

	bs = (struct ev_bsock *)w->data;
	if (state == APP_READ) {
		rc = bsock_tls_read(bs, bs->rbuf->start, buff_left(bs->rbuf));
		if (rc < 0) {
			bsock_destroy(EV_A_ bs, BS_F_KEP);
			//bsock_destroy(EV_A_ bs, 0);
			return;
		} else if (rc == 0) {
			return;
		}
		assert(ev_is_active(&bs->wsock) == 0);
		assert(ev_is_active(&bs->rsock) == 1);
		bs->rbuf->tail = bs->rbuf->start + rc - 1; /* ignore '\n' */
		buff_seal(bs->rbuf);
		log_info(MOD "got data: %s", buff_string(bs->rbuf));

		ev_io_stop(EV_A_ &bs->rsock);
		ev_io_start(EV_A_ &bs->wsock);
		state = APP_WRITE;
	} else if (state == APP_WRITE) {
		rc = bsock_tls_write(bs, bs->rbuf->start, buff_length(bs->rbuf));
		if (rc < 0) {
			bsock_destroy(EV_A_ bs, BS_F_KEP);
			//bsock_destroy(EV_A_ bs, 0);
			return;
		} else if (rc == 0) {
			return;
		}
		assert(ev_is_active(&bs->wsock) == 1);
		assert(ev_is_active(&bs->rsock) == 0);
		log_info(MOD "snd data: %s", buff_string(bs->rbuf));

		ev_io_stop(EV_A_ &bs->wsock);
		ev_io_start(EV_A_ &bs->rsock);
		state = APP_READ;
	}
}

static void server_event(struct ev_bsock *bs, int event, void *arg)
{
	(void) arg;
	log_info(MOD "got event <%s>.", bsock_get_event(event));

	switch (event) {
	case BS_EV_SSL_END: /* SSL/TLS handshake finished */
		if (bsock_tls_verify_certs(bs, bs->tls->ssl) < 0) {
			bsock_destroy(EV_A_ bs, 0);
			return;
		}
		bsock_tls_showinfo(bs, bs->tls->ssl);

		ev_io_init(&bs->rsock, server_app_process_hook, bs->fd, EV_READ);
		ev_io_init(&bs->wsock, server_app_process_hook, bs->fd, EV_WRITE);

		ev_io_start(EV_A_ &bs->rsock);
		state = APP_READ;
		break;

	default:
		log_warning(MOD "unsupport event.");
	}
}

static void sigpipe_handler_hook(EV_P_ ev_signal *w, int revents)
{
	(void) w;
	(void) revents;
	log_info(MOD "got SIGPIPE.");
}


static void sigint_handler_hook(EV_P_ ev_signal *w, int revents)
{
	(void) w;
	(void) revents;
	log_info(MOD "got SIGINT, leave now.");
	ev_break(EV_A_ EVBREAK_ALL);
}

int main(void)
{
	struct ev_bsock *sock;
	struct socketinfo info;
	struct bsock_cert cert = {
		.key    = "pki/SVR/key.pem",
		.cert   = "pki/SVR/cert.pem",
		.cacert = "pki/CA/cacert.pem",
	};

	struct ev_signal wsigint, wsigpipe;
	EV_DEFAULT_DEC; /* OR ev_default_loop(0) */

	log_init(NULL, LOG_TRACE);

	if (bsock_ssl_init() < 0) {
		log_error(MOD "init ssl module failed.");
		return -1;
	}

	sock = bsock_create(4096, 4096, BS_F_SSL | BS_F_CTX | BS_F_SVR);
	if (sock == NULL)
		return -1;
	bsock_set_event(sock, server_event);
	//SSL_CTX_set_options(sock->tls->ctx, SSL_OP_NO_TLSv1_3);

	if (bsock_tls_setup_cert(sock, &cert) < 0) {
		bsock_destroy(EV_A_ sock, 0);
		return -1;
	}

#if 0
	/* set our supported ciphers */
	if(SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH") != 1) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
#endif

	/* the client have to send it's certificate, default SSL_VERIFY_NONE */
	SSL_CTX_set_verify(sock->tls->ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	//SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

	memset(&info, 0, sizeof(info));
	info.family = AF_INET;
	info.ipv4.sin_family = AF_INET;
	info.ipv4.sin_port = htons(8090);
	info.ipv4.sin_addr.s_addr = INADDR_ANY;
	bsock_set_socket(sock, &info);

	if (bsock_listen(sock) < 0) {
		bsock_destroy(EV_A_ sock, 0);
		return -1;
	}

	ev_signal_init(&wsigint, sigint_handler_hook, SIGINT);
	ev_signal_start(EV_A_ &wsigint);

	ev_signal_init(&wsigpipe, sigpipe_handler_hook, SIGPIPE);
	ev_signal_start(EV_A_ &wsigpipe);

        ev_run(EV_A_ 0);

	bsock_destroy(EV_A_ sock, 0);
	ev_loop_destroy(EV_A);

	return 0;
}
