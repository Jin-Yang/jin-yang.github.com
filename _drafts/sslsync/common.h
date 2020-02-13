#ifndef COMMON_H_
#define COMMON_H_

#include <unistd.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "libev/ev.h"

#define likely
#define unlikely

enum ss_state {
        SS_INVALID,
        SS_CONNECTING,
        SS_SSL_HANDSHAKE,
        SS_HANDSHAKE,
        SS_APP_READ,
        SS_APP_WRITE,
};

enum ss_role {
        SS_R_NONE,
        SS_R_SERVER,
        SS_R_CLIENT,
};

struct ss_certinfo {
        const char *cacert;
        const char *key;
        const char *cert;
};

struct ss_context {
        enum ss_role role;
        enum ss_state state;

        int sockfd;
        struct ev_timer wtimeout;
        struct ev_io wsock, rsock;

        struct {
                SSL *ssl;
                SSL_CTX *ctx;
                void(*hook)(EV_P_ struct ss_context *);
        } ssl;

        struct {
                char *body;
                int len, max;
        } buff;
        struct sockaddr_in addr;
};

struct ss_context *ssock_context_create(int size);
void ssock_context_reset(EV_P_ struct ss_context *ctx);
void ssock_context_destroy(EV_P_ struct ss_context *ctx);

SSL_CTX *ssock_ssl_init(int flag);
int ssock_ssl_prepre(SSL_CTX *ctx, struct ss_certinfo *cert);
SSL *ssock_ssl_create(SSL_CTX *ctx, int sockfd, int flag);
void ssock_ssl_handshake(EV_P_ struct ss_context *ctx);

int ssock_verify_show_certs(SSL *ssl);

int ssock_read(struct ss_context *ctx, char *buff, int len);
int ssock_write(struct ss_context *ctx, char *buff, int len);

#endif
