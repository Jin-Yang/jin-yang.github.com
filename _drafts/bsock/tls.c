#include <assert.h>

#include "bsock.h"

#define MOD "(sock) "
#define MODP "(sock) [%s:%s %s] "
#define MODA bs->ipaddr, bs->ipport, BS_STATE[bs->state]

#define TLS_ERR_SIZE  4096

extern const char *BS_STATE[BS_S_MAX];

#if 0
void ngx_cdecl
ngx_ssl_error(ngx_uint_t level, ngx_log_t *log, ngx_err_t err, char *fmt, ...)
{
    int          flags;
    u_long       n;
    va_list      args;
    u_char      *p, *last;
    u_char       errstr[NGX_MAX_CONF_ERRSTR];
    const char  *data;

    last = errstr + NGX_MAX_CONF_ERRSTR;

    va_start(args, fmt);
    p = ngx_vslprintf(errstr, last - 1, fmt, args);
    va_end(args);

    p = ngx_cpystrn(p, (u_char *) " (SSL:", last - p);

    for ( ;; ) {

        n = ERR_peek_error_line_data(NULL, NULL, &data, &flags);

        if (n == 0) {
            break;
        }

        if (p >= last) {
            goto next;
        }

        *p++ = ' ';

        ERR_error_string_n(n, (char *) p, last - p);

        while (p < last && *p) {
            p++;
        }

        if (p < last && *data && (flags & ERR_TXT_STRING)) {
            *p++ = ':';
            p = ngx_cpystrn(p, (u_char *) data, last - p);
        }

    next:

        (void) ERR_get_error();
    }

    ngx_log_error(level, log, err, "%s)", errstr);
}
#endif

static void bsock_log_sslerr(char *fmt, ...)
{
	int rc;
	va_list args;
	char errstr[TLS_ERR_SIZE], *ptr;

	va_start(args, fmt);
	rc = vsnprintf(errstr, sizeof(errstr), fmt, args);
	va_end(args);
	assert(rc >= 0);
	if (rc >= (int)sizeof(errstr) - 6) {
		errstr[sizeof(errstr - 1)] = 0;
		log_error("%s", errstr);
		return;
	}
	/* simple not safe. */
	//log_error("%s(SSL: %s).", errstr, ERR_error_string(ERR_get_error(), NULL));

	ptr = errstr + rc;
	memcpy(ptr, "(SSL: ", 6);
}

static void bsock_openssl_detail_info(const SSL *ssl, int where, int ret)
{
	int w;
	const char *str;

	w = where & ~SSL_ST_MASK;
	if (w & SSL_ST_CONNECT)
		str = "SSL_connect";
	else if (w & SSL_ST_ACCEPT)
		str = "SSL_accept";
	else
		str = "SSL_undefined";

	if (where & SSL_CB_LOOP) {
		log_debug(MOD "%s:%s", str, SSL_state_string_long(ssl));
		return;
	} else if (where & SSL_CB_ALERT) {
		str = (where & SSL_CB_READ) ? "read" : "write";
		log_error(MOD "SSL3 alert %s:%s:%s", str,
				SSL_alert_type_string_long(ret),
				SSL_alert_desc_string_long(ret));
		return;
	} else if (where & SSL_CB_EXIT) {
		if (errno == EAGAIN || errno == EINTR)
			return;
		if (ret == 1)
			return;

		if (ret == 0) {
			log_error(MOD "%s:failed in state %s",
				str, SSL_state_string_long(ssl));
		} else if (ret < 0) {
			log_error(MOD "%s:error in state %s",
				str, SSL_state_string_long(ssl));
		}
		return;
	}
}

#if 0
static void bsock_openssl_simple_info(const SSL *ssl, int where, int ret)
{
	int logit = 0; /* ignore low-level SSL stuff */

	if (where & SSL_CB_ALERT)
		logit = 1;
	if (where == SSL_CB_HANDSHAKE_START || where == SSL_CB_HANDSHAKE_DONE)
		logit = 1;
	if ((where & SSL_CB_EXIT) && ret == 0)
		logit = 1;
	if (logit == 0)
		return;

	fprintf(stderr, "0x%04x %s, return 0x%x.\n", where, SSL_state_string_long(ssl), ret);
	if (where == SSL_CB_HANDSHAKE_DONE)
		fprintf(stderr, "using SSL version %s cipher=%s\n", SSL_get_version(ssl), SSL_get_cipher_name(ssl));
}
#endif

int bsock_ssl_init(void)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms(); //OpenSSL_add_ssl_algorithms();
#else
	if (OPENSSL_init_ssl(OPENSSL_INIT_ENGINE_ALL_BUILTIN | OPENSSL_INIT_LOAD_CONFIG, NULL) == 0) {
		log_error(MOD "init OpenSSL failed.");
		return -1;
	}
#endif

	return 0;
}

SSL *bsock_ssl_create(SSL_CTX *ctx, int sockfd, int flag)
{
	SSL *ssl;

	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		log_error(MOD "create SSL failed.");
		return NULL;
	}

	if (SSL_set_fd(ssl, sockfd) == 0) {
		log_error(MOD "set SSL fd failed, %s.",
				ERR_reason_error_string(ERR_get_error()));
		SSL_free(ssl);
		return NULL;
	}
	//SSL_set_mode(ctx->ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);

	if (flag & BS_F_SVR)
		SSL_set_accept_state(ssl);
	else
		SSL_set_connect_state(ssl);

	return ssl;
}

SSL_CTX *bsock_ssl_create_context(int flag)
{
	SSL_CTX *ctx;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (flag & BS_F_SVR)
		ctx = SSL_CTX_new(SSLv23_server_method());
	else
		ctx = SSL_CTX_new(SSLv23_client_method());
#else
	if (flag & BS_F_SVR)
		ctx = SSL_CTX_new(TLS_server_method());
	else
		ctx = SSL_CTX_new(TLS_client_method());
#endif
	if (ctx == NULL) {
		//ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return NULL;
	}

	//SSL_CTX_set_msg_callback(ctx, SSL_trace);
	SSL_CTX_set_info_callback(ctx, bsock_openssl_detail_info);
	//SSL_CTX_set_info_callback(ctx, bsock_openssl_simple_info);

	return ctx;
}

void bsock_tls_showinfo(struct ev_bsock *bs, SSL *ssl)
{
        X509 *cert;
        char *line;
	const SSL_CIPHER *c;

        cert = SSL_get_peer_certificate(ssl);
        if (cert == NULL) {
		log_error(MODP "no certificate information got!", MODA);
                return;
        }
	log_info(MODP "----> %s connection.", MODA, SSL_session_reused(ssl) ? "reused" : "new");
	log_info(MODP "digital certificate information:", MODA);

        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
	if (line != NULL) {
		log_info(MODP "certificate: %s", MODA, line);
		free(line);
	}

        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
	if (line != NULL) {
		log_info(MODP "issuer: %s", MODA, line);
		free(line);
	}
        X509_free(cert);

	c = SSL_get_current_cipher(ssl);
	log_info(MODP "%s, cipher is %s.", MODA, SSL_CIPHER_get_version(c), SSL_CIPHER_get_name(c));
}

int bsock_tls_verify_certs(struct ev_bsock *bs, SSL *ssl)
{
        X509 *cert;

        cert = SSL_get_peer_certificate(ssl);
        if (cert == NULL) {
		log_error(MODP "no certificate information got!", MODA);
                return -1;
        }

        if (SSL_get_verify_result(ssl) != X509_V_OK){
		log_error(MODP "invalid certificate information!", MODA);
                return -1;
        }
        X509_free(cert);

        return 0;
}

int bsock_tls_setup_cert(struct ev_bsock *bs, struct bsock_cert *cert)
{
	SSL_CTX *ctx;

	if (bs == NULL || cert == NULL || bs->tls == NULL || bs->tls->ctx == NULL)
		return -EINVAL;
	ctx = bs->tls->ctx;

	if (cert->cacert) { /* load ca file */
		if (SSL_CTX_load_verify_locations(ctx, cert->cacert, 0) != 1) {
			bsock_log_sslerr(MOD "load CA file '%s' failed", cert->cacert);
			return -1;
		}

		if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
			bsock_log_sslerr(MOD "set default verify path failed.");
			return -1;
		}
	}

	if (cert->key == NULL && cert->cert == NULL)
		return 0;

        /* load certificate file, which will send public key to client. */
	if (SSL_CTX_use_certificate_file(ctx, cert->cert, SSL_FILETYPE_PEM) != 1) {
		bsock_log_sslerr(MOD "set certificate file '%s' failed.", cert->cert);
		return -1;
	}

	/* and also private key file */
	if (SSL_CTX_use_PrivateKey_file(ctx, cert->key, SSL_FILETYPE_PEM) != 1) {
		bsock_log_sslerr(MOD "set private key '%s' failed.", cert->key);
		return -1;
	}

	/* check if the private key is valid */
	if (SSL_CTX_check_private_key(ctx) != 1) {
		bsock_log_sslerr(MOD "check private key failed.");
		return -1;
	}

	return 0;
}

void bsock_ssl_handshake(EV_P_ struct ev_bsock *bs)
{
	int rc, err;

	//ERR_clear_error();
	rc = SSL_do_handshake(bs->tls->ssl);
	if (rc == 1) {
		log_info(MODP "SSL handshake was successfully completed.", MODA);
		bsock_reset(EV_A_ bs);
		if (bs->event)
			bs->event(bs, BS_EV_SSL_END, bs->arg);
		else
			bsock_destroy(EV_A_ bs, 0);
		return;
	} else if (rc == 0) {
		log_error(MODP "SSL handshake was not successful but was shutdown.", MODA);
		bsock_destroy(EV_A_ bs, 0);
		return;
	}

	err = SSL_get_error(bs->tls->ssl, rc);
	if (err == SSL_ERROR_WANT_READ) {
		log_trace(MODP "SSL want read.", MODA);
		if (ev_is_active(&bs->wsock) == 1)
			ev_io_stop(EV_A_ &bs->wsock);
		if (ev_is_active(&bs->rsock) == 0)
			ev_io_start(EV_A_ &bs->rsock);
		return;
	} else if (err == SSL_ERROR_WANT_WRITE) {
		log_trace(MODP "SSL want write.", MODA);
		if (ev_is_active(&bs->rsock) == 1)
			ev_io_stop(EV_A_ &bs->rsock);
		if (ev_is_active(&bs->wsock) == 0)
			ev_io_start(EV_A_ &bs->wsock);
		return;
	} else if (err == SSL_ERROR_SSL) {
		log_error(MODP "bad SSL peer.", MODA);
		bsock_destroy(EV_A_ bs, 0);
		return;
	} else if (err == SSL_ERROR_SYSCALL && rc == 0) {
		log_error(MODP "peer SSL client maybe closed.", MODA);
		bsock_destroy(EV_A_ bs, 0);
		return;
	}

	log_error(MODP "SSL do handshake failed, %d:%s.", MODA, errno, strerror(errno));
	bsock_destroy(EV_A_ bs, 0);
}

void bsock_tls_shutdown(SSL *ssl)
{
	int rc;

	do {
		/* We only do unidirectional shutdown */
		rc = SSL_shutdown(ssl);
		if (rc < 0) {
			switch (SSL_get_error(ssl, rc)) {
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
				case SSL_ERROR_WANT_ASYNC:
				case SSL_ERROR_WANT_ASYNC_JOB:
				/* We just do busy waiting. Nothing clever */
				continue;
			}
			rc = 0;
		}
	} while (rc < 0);
}

static const char *bsock_get_errcode(int errcode)
{
	switch (errcode) {
	case SSL_ERROR_NONE:
		return "SSL_ERROR_NONE";
	case SSL_ERROR_ZERO_RETURN:
		return "SSL_ERROR_ZERO_RETURN";
	case SSL_ERROR_WANT_READ:
		return "SSL_ERROR_WANT_READ";
	case SSL_ERROR_WANT_WRITE:
		return "SSL_ERROR_WANT_WRITE";
	case SSL_ERROR_WANT_CONNECT:
		return "SSL_ERROR_WANT_CONNECT";
	case SSL_ERROR_WANT_ACCEPT:
		return "SSL_ERROR_WANT_ACCEPT";
	case SSL_ERROR_WANT_X509_LOOKUP:
		return "SSL_ERROR_WANT_X509_LOOKUP";
	case SSL_ERROR_SYSCALL:
		return "SSL_ERROR_SYSCALL";
	case SSL_ERROR_SSL:
		return "SSL_ERROR_SSL";
	default:
		return "UNKNOWN";
	}
	return "UNKNOWN";
}

int bsock_tls_read(struct ev_bsock *bs, char *buff, int len)
{
	SSL *ssl;
	int rc, err;

	assert(bs->tls != NULL && bs->tls->ssl != NULL);
	ssl = bs->tls->ssl;
	rc = SSL_read(ssl, buff, len);
	if (rc > 0) /* fast path */
		return rc;
	err = SSL_get_error(ssl, rc);
	log_trace(MODP "got read error code <%s/%d>, return code %d.", MODA,
			bsock_get_errcode(err), err, rc);
	switch (err) {
	case SSL_ERROR_SYSCALL:
		if (rc == 0)
			log_error(MODP "peer SSL client maybe closed.", MODA);
		else
			log_error(MODP "peer SSL client error, %d:%s.", MODA, errno, strerror(errno));
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		log_error(MODP "peer SSL client closed.", MODA);
		return  -1;
	case SSL_ERROR_WANT_X509_LOOKUP:
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		log_trace(MODP "event read %d, write %d.", MODA,
				ev_is_active(&bs->rsock), ev_is_active(&bs->wsock));
		return 0;
	//case SSL_ERROR_WANT_ASYNC: /* not support */
	}

	//log_error(MODP "got read SSL error, %s.", MODA, ERR_reason_error_string(err));
	log_error(MODP "got read SSL error, %s.", MODA, ERR_error_string(err, NULL));
	return -1;
#if 0
        int rc;

        rc = read(ctx->sockfd, buff, len);
        if (rc < 0) {
                fprintf(stderr, "read from socket %d failed, %d:%s.\n", ctx->sockfd,
                        errno, strerror(errno));
                return rc;
        } else if (rc == 0) {
                fprintf(stderr, "peer maybe closed.\n");
                return -1;
        }
        return rc;
#endif
}

int bsock_tls_write(struct ev_bsock *bs, char *buff, int len)
{
	SSL *ssl;
	int rc, err;

	assert(bs->tls != NULL && bs->tls->ssl != NULL);
	ssl = bs->tls->ssl;
	rc = SSL_write(ssl, buff, len);
	if (rc > 0)
		return rc;

	err = SSL_get_error(ssl, rc);
	log_trace(MODP "got write error code <%s/%d>, return code %d.", MODA,
			bsock_get_errcode(err), err, rc);
	switch (err) {
	case SSL_ERROR_SYSCALL:
		if (rc == 0)
			log_error(MODP "peer SSL client maybe closed.", MODA);
		else
			log_error(MODP "peer SSL client error, %d:%s.", MODA, errno, strerror(errno));
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		log_error(MODP "peer SSL client closed.", MODA);
		return  -1;
	case SSL_ERROR_WANT_X509_LOOKUP:
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		return 0;
	//case SSL_ERROR_WANT_ASYNC: /* not support */
	}

	log_error(MODP "got write SSL error, %s.", MODA, ERR_error_string(err, NULL));
	return -1;

#if 0
        int rc;

        rc = write(ctx->sockfd, buff, len);
        if (rc < 0) {
                fprintf(stderr, "write to socket %d failed, %d:%s.\n", ctx->sockfd,
                        errno, strerror(errno));
                return rc;
        } else if (rc == 0) {
                fprintf(stderr, "peer maybe closed.\n");
                return -1;
        }
        return rc;
#endif
}
