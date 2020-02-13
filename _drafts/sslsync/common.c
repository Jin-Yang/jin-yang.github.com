#include "common.h"

#include <errno.h>

#define USE_SSL   1

struct ss_context *ssock_context_create(int size)
{
        struct ss_context *ctx;

        ctx = (struct ss_context *)calloc(1, sizeof(*ctx));
        if (ctx == NULL)
                return NULL;
        ctx->sockfd = -1;

        ctx->buff.max = size;
        ctx->buff.body = malloc(ctx->buff.max);
        if (ctx->buff.body == NULL) {
                free(ctx);
                return NULL;
        }

        ctx->rsock.data = ctx;
        ctx->wsock.data = ctx;
        ctx->wtimeout.data = ctx;

        return ctx;
}

void ssock_context_reset(EV_P_ struct ss_context *ctx)
{
        if (ctx == NULL)
                return;
        if (ev_is_active(&ctx->rsock))
                ev_io_stop(EV_A_ &ctx->rsock);
        if (ev_is_active(&ctx->wsock))
                ev_io_stop(EV_A_ &ctx->wsock);
        if (ev_is_active(&ctx->wtimeout))
                ev_timer_stop(EV_A_ &ctx->wtimeout);
        if (ctx->sockfd >= 0)
                close(ctx->sockfd);
        ctx->sockfd = -1;

        if (ctx->ssl.ssl != NULL) {
                SSL_shutdown(ctx->ssl.ssl);
                SSL_free(ctx->ssl.ssl);
        }
        ctx->ssl.ssl = NULL;

        if (ctx->buff.body != NULL)
                free(ctx->buff.body);
        ctx->buff.body = NULL;
}

void ssock_context_destroy(EV_P_ struct ss_context *ctx)
{
        if (ctx == NULL)
                return;

        ssock_context_reset(EV_A_ ctx);
        free(ctx);
}

void ssock_openssl_detail_info(const SSL *ssl, int where, int ret)
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
                fprintf(stderr, "%s:%s\n", str, SSL_state_string_long(ssl));
                return;
        } else if (where & SSL_CB_ALERT) {
                str = (where & SSL_CB_READ) ? "read" : "write";
                fprintf(stderr, "SSL3 alert %s:%s:%s\n", str,
                                SSL_alert_type_string_long(ret),
                                SSL_alert_desc_string_long(ret));
                return;
        } else if (where & SSL_CB_EXIT) {
                if (errno == EAGAIN || errno == EINTR)
                        return;
                if (ret == 1)
                        return;

                if (ret == 0) {
                        fprintf(stderr, "%s:failed in state %s\n",
                                str, SSL_state_string_long(ssl));
                } else if (ret < 0) {
                        fprintf(stderr, "%s:error in state %s\n",
                                str, SSL_state_string_long(ssl));
                }
                return;
        }
}
void ssock_openssl_simple_info(const SSL *ssl, int where, int ret)
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

SSL_CTX *ssock_ssl_init(int flag)
{
        SSL_CTX *ctx;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms(); //OpenSSL_add_ssl_algorithms();
#else
	if (OPENSSL_init_ssl(OPENSSL_INIT_ENGINE_ALL_BUILTIN | OPENSSL_INIT_LOAD_CONFIG, NULL) == 0) {
		fprintf(stderr, "init OpenSSL failed.\n");
		return NULL;
	}
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
        if (flag == SS_R_SERVER)
                ctx = SSL_CTX_new(SSLv23_server_method());
        else // flag == SS_R_CLIENT
                ctx = SSL_CTX_new(SSLv23_client_method());
#else
        if (flag == SS_R_SERVER)
                ctx = SSL_CTX_new(TLS_server_method());
        else // flag == SS_R_CLIENT
                ctx = SSL_CTX_new(TLS_client_method());
#endif
        if (ctx == NULL) {
                ERR_print_errors_fp(stderr);
                SSL_CTX_free(ctx);
                return NULL;
        }
        //SSL_CTX_set_msg_callback(ctx, SSL_trace);
        SSL_CTX_set_info_callback(ctx, ssock_openssl_detail_info);
        //SSL_CTX_set_info_callback(ctx, ssock_openssl_simple_info);

        return ctx;
}

int ssock_ssl_prepre(SSL_CTX *ctx, struct ss_certinfo *cert)
{
        if (ctx == NULL || cert == NULL)
                return -EINVAL;

        /* load ca file */
        if (cert->cacert) {
                if (SSL_CTX_load_verify_locations(ctx, cert->cacert, 0) != 1) {
                        ERR_print_errors_fp(stderr);
                        return -1;
                }
                if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
                        ERR_print_errors_fp(stderr);
                        return -1;
                }
        }

        if (cert->key == NULL && cert->cert == NULL)
                return 0;

        /* load certificate file, which will send public key to client. */
        if (SSL_CTX_use_certificate_file(ctx, cert->cert, SSL_FILETYPE_PEM) != 1) {
                ERR_print_errors_fp(stderr);
                return -1;
        }
        /* and also private key file */
        if (SSL_CTX_use_PrivateKey_file(ctx, cert->key, SSL_FILETYPE_PEM) != 1) {
                ERR_print_errors_fp(stderr);
                return -1;
        }
        /* check if the private key is valid */
        if (SSL_CTX_check_private_key(ctx) != 1) {
                ERR_print_errors_fp(stderr);
                return -1;
        }

        return 0;
}

SSL *ssock_ssl_create(SSL_CTX *ctx, int sockfd, int flag)
{
        SSL *ssl;

        ssl = SSL_new(ctx);
        if (ssl == NULL) {
                fprintf(stderr, "create SSL failed.\n");
                return NULL;
        }

        if (SSL_set_fd(ssl, sockfd) == 0) {
                fprintf(stderr, "set SSL fd failed, %s.\n",
                        ERR_reason_error_string(ERR_get_error()));
                SSL_free(ssl);
                return NULL;
        }
        //SSL_set_mode(ctx->ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);

        if (flag == SS_R_SERVER)
                SSL_set_accept_state(ssl);
        else if (flag == SS_R_CLIENT)
                SSL_set_connect_state(ssl);

        return ssl;
}

int ssock_verify_show_certs(SSL *ssl)
{
        X509 *cert;
        char *line;

        cert = SSL_get_peer_certificate(ssl);
        if (cert == NULL) {
                printf("No certificate information!\n");
                return -1;
        }

        if (SSL_get_verify_result(ssl) != X509_V_OK){
                printf("Invalid certificate information!\n");
                return -1;
        }
        printf("Digital certificate information:\n");

        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Certificate: %s\n", line);
        free(line);

        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("     Issuer: %s\n", line);
        free(line);

        X509_free(cert);
        return 0;
}

void ssock_ssl_handshake(EV_P_ struct ss_context *ctx)
{
        int rc, err;

        ERR_clear_error();
        rc = SSL_do_handshake(ctx->ssl.ssl);
        if (rc == 1) {
                fprintf(stderr, "TLS/SSL handshake was successfully completed, %d.\n", SSL_is_init_finished(ctx->ssl.ssl));
                if (ctx->ssl.hook != NULL)
                        ctx->ssl.hook(EV_A_ ctx);
                else
                        ssock_context_destroy(EV_A_ ctx);
                return;
        } else if (rc == 0) {
                fprintf(stderr, "TLS/SSL handshake was not successful but was shutdown.\n");
                ssock_context_destroy(EV_A_ ctx);
                return;
        }

        err = SSL_get_error(ctx->ssl.ssl, rc);
        if (err == SSL_ERROR_WANT_READ) {
                fprintf(stderr, "TLS/SSL want read.\n");
                if (ev_is_active(&ctx->wsock) == 1)
                        ev_io_stop(EV_A_ &ctx->wsock);
                if (ev_is_active(&ctx->rsock) == 0)
                        ev_io_start(EV_A_ &ctx->rsock);
                return;
        } else if (err == SSL_ERROR_WANT_WRITE) {
                fprintf(stderr, "TLS/SSL want write.\n");
                if (ev_is_active(&ctx->rsock) == 1)
                        ev_io_stop(EV_A_ &ctx->rsock);
                if (ev_is_active(&ctx->wsock) == 0)
                        ev_io_start(EV_A_ &ctx->wsock);
                return;
        } else if (err == SSL_ERROR_SSL) {
                fprintf(stderr, "bad TLS/SSL peer.\n");
                ssock_context_destroy(EV_A_ ctx);
                return;
        } else if (err == SSL_ERROR_SYSCALL && rc == 0) {
                fprintf(stderr, "peer TLS/SSL client maybe closed.\n");
                ssock_context_destroy(EV_A_ ctx);
                return;
        }

        fprintf(stderr, "SSL do handshake failed, %d:%s.\n", errno, strerror(errno));
        ssock_context_destroy(EV_A_ ctx);
}

int ssock_read(struct ss_context *ctx, char *buff, int len)
{
#if USE_SSL
        int rc, err;

        rc = SSL_read(ctx->ssl.ssl, buff, len);
	err = SSL_get_error(ctx->ssl.ssl, rc);
        if (rc > 0) {
                return rc;
        } else if (rc < 0) {
                /* SSL_ERROR_SYSCALL SSL_ERROR_SSL */
                err = SSL_get_error(ctx->ssl.ssl, rc);
                if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
                        return 0;
                fprintf(stderr, "got SSL error, %s\n",
                                ERR_reason_error_string(SSL_get_error(ctx->ssl.ssl, rc)));
                return rc;
        } else if (rc == 0) {
                err = SSL_get_error(ctx->ssl.ssl, 0);
                if (err == SSL_ERROR_SYSCALL) {
                        fprintf(stderr, "peer SSL client maybe closed.\n");
                        return -1;
                }
                fprintf(stderr, "got SSL error, %s\n",
                                ERR_reason_error_string(SSL_get_error(ctx->ssl.ssl, rc)));
                return -1;
        }
        return rc;
#else
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

int ssock_write(struct ss_context *ctx, char *buff, int len)
{
#if USE_SSL
        int rc, err;

        rc = SSL_write(ctx->ssl.ssl, buff, len);
        if (rc > 0) {
                return rc;
        } else if (rc < 0) {
                /* SSL_ERROR_SYSCALL SSL_ERROR_SSL */
                err = SSL_get_error(ctx->ssl.ssl, rc);
                if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
                        return 0;
                fprintf(stderr, "got SSL error, %s\n",
                                ERR_reason_error_string(SSL_get_error(ctx->ssl.ssl, rc)));
                return rc;
        } else if (rc == 0) {
                err = SSL_get_error(ctx->ssl.ssl, 0);
                if (err == SSL_ERROR_SYSCALL) {
                        fprintf(stderr, "peer SSL client maybe closed.\n");
                        return -1;
                }
                fprintf(stderr, "got SSL error, %s\n",
                                ERR_reason_error_string(SSL_get_error(ctx->ssl.ssl, rc)));
                return -1;
        }
        return rc;
#else
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
