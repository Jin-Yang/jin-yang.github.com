#include <assert.h>

#include "libev/ev.h"
#include "../../socket/common.h"
#include "../../socket/bsock/bsock.h"

#define MOD "(main) "

#define USE_KEY_PASS 1

static int count = 1;
enum {
	APP_INIT,
	APP_READ,
	APP_WRITE,
} state;

static void client_app_process_hook(EV_P_ struct ev_io *w, int revents)
{
	(void) revents;
	int rc;
	struct ev_bsock *bs;

	bs = (struct ev_bsock *)w->data;
	if (state == APP_READ) {
		rc = bsock_tls_read(bs, bs->rbuf->start, buff_left(bs->rbuf));
		if (rc < 0) {
			bsock_destroy(EV_A_ bs, 0);
			return;
		} else if (rc == 0) {
			return;
		}
		assert(ev_is_active(&bs->wsock) == 0);
		assert(ev_is_active(&bs->rsock) == 1);
		bs->rbuf->tail = bs->rbuf->start + rc;
		buff_seal(bs->rbuf);
		log_info(MOD "got data: %s", buff_string(bs->rbuf));

		if (--count <= 0) {
			log_info(MOD "leave now.");
			bsock_destroy(EV_A_ bs, 0);
			return;
		}
		bsock_close(EV_A_ bs, 0);
		SSL_set_connect_state(bs->tls->ssl);
		bsock_connect(bs);
		return;
	} else if (state == APP_WRITE) {
		rc = bsock_tls_write(bs, bs->wbuf->start, buff_length(bs->wbuf));
		if (rc < 0) {
			bsock_destroy(EV_A_ bs, 0);
			return;
		} else if (rc == 0) {
			return;
		}

		assert(ev_is_active(&bs->wsock) == 1);
		assert(ev_is_active(&bs->rsock) == 0);
		ev_io_stop(EV_A_ &bs->wsock);
		ev_io_start(EV_A_ &bs->rsock);
		buff_seal(bs->wbuf);
		log_info(MOD "snd data: %s", buff_string(bs->wbuf));
		buff_restart(bs->wbuf);
		state = APP_READ;
	}
}

static void client_event(struct ev_bsock *bs, int event, void *arg)
{
	(void) arg;
	log_info(MOD "client got event <%s>.", bsock_get_event(event));
	switch (event) {
	case BS_EV_SSL_END: /* SSL/TLS handshake finished */
		if (bsock_tls_verify_certs(bs, bs->tls->ssl) < 0) {
			bsock_destroy(EV_A_ bs, 0);
			return;
		}
		bsock_tls_showinfo(bs, bs->tls->ssl);

		ev_io_init(&bs->rsock, client_app_process_hook, bs->fd, EV_READ);
		ev_io_init(&bs->wsock, client_app_process_hook, bs->fd, EV_WRITE);

#define HIMSG "Hello World!\n"
		memcpy(bs->wbuf->start, HIMSG, sizeof(HIMSG) - 1);
		bs->wbuf->tail = bs->wbuf->start + (sizeof(HIMSG) - 1);
		ev_io_start(EV_A_ &bs->rsock);
		//state = APP_WRITE;
		break;

	default:
		log_warning(MOD "unsupport event.");

	}
}

static int client_new_session_hook(SSL *s, SSL_SESSION *sess)
{
	(void) sess;
	/*
	** Session data gets dumped on connection for TLSv1.2 and below, and on
	** arrival of the NewSessionTicket for TLSv1.3.
	*/
	if (SSL_version(s) == TLS1_3_VERSION) {
		log_info(MOD "Post-Handshake New Session Ticket arrived.");
#if 0
		SSL_SESSION_print(bio_c_out, sess);
		BIO_printf(bio_c_out, "---\n");
#endif
	}

	/*
	** We always return a "fail" response so that the session gets freed again
	** because we haven't used the reference.
	*/
	return 0;
}

#if USE_KEY_PASS
static int client_password_callback(char *buf, int size, int rwflag, void *userdata)
{
        (void) userdata;

	log_trace(MOD "invoking passwork callback, size %d flag %d.", size, rwflag);
	if (rwflag == 1)
		return -1;

#define PASS "123456"
        if ((int)sizeof(PASS) - 1 >= size) {
		log_error(MOD "password is too long, larger than %d bytes.", size);
                return -1;
        }
        strncpy(buf, PASS, (int)sizeof(PASS)); /* including NULL term */

        return sizeof(PASS) - 1;
}
#endif

static void sigpipe_handler_hook(EV_P_ ev_signal *w, int revents)
{
	(void) w;
	(void) revents;
	log_info(MOD "got SIGPIPE.");
}

int main(void)
{
	struct ev_bsock *sock;
	struct socketinfo info;
	struct bsock_cert cert = {
#if USE_KEY_PASS
		.key    = "pki/CLI/keysec.pem",
		.cert   = "pki/CLI/certsec.pem",
		.cacert = "pki/CA/cacert.pem",
#else
		.key    = "pki/CLI/key.pem",
		.cert   = "pki/CLI/cert.pem",
		.cacert = "pki/CA/cacert.pem",
#endif
	};
	struct ev_signal wsigpipe;
	EV_DEFAULT_DEC; /* OR ev_default_loop(0) */

	log_init(NULL, LOG_TRACE);

	ev_signal_init(&wsigpipe, sigpipe_handler_hook, SIGPIPE);
	ev_signal_start(EV_A_ &wsigpipe);

	if (bsock_ssl_init() < 0) {
		log_error(MOD "init ssl module failed.");
		return -1;
	}

	sock = bsock_create(4096, 4096, BS_F_SSL | BS_F_CTX);
	if (sock == NULL)
		return -1;
	bsock_set_event(sock, client_event);

	SSL_CTX_set_session_cache_mode(sock->tls->ctx,
		SSL_SESS_CACHE_CLIENT | SSL_SESS_CACHE_NO_INTERNAL_STORE);
	SSL_CTX_sess_set_new_cb(sock->tls->ctx, client_new_session_hook);

#if USE_KEY_PASS
        SSL_CTX_set_default_passwd_cb_userdata(sock->tls->ctx, NULL);
        SSL_CTX_set_default_passwd_cb(sock->tls->ctx, client_password_callback);
#endif

	if (bsock_tls_setup_cert(sock, &cert) < 0) {
		bsock_destroy(EV_A_ sock, 0);
		ev_loop_destroy(EV_A);
		return -1;
	}

	memset(&info, 0, sizeof(info));
	info.family = AF_INET;
	info.ipv4.sin_family = AF_INET;
	info.ipv4.sin_port = htons(8090);
	info.ipv4.sin_addr.s_addr = INADDR_ANY;
        inet_aton("127.0.0.1", (struct in_addr *)&info.ipv4.sin_addr.s_addr);
	bsock_set_socket(sock, &info);

	bsock_connect(sock);

	ev_run(EV_A_ 0);

	ev_loop_destroy(EV_A);

	return 0;
}
