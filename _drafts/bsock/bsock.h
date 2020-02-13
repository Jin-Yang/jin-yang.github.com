#ifndef LIBDOWN_BSOCK_H_
#define LIBDOWN_BSOCK_H_

#include "../common.h"
#include "libev/ev.h"
#include "liblog/log.h"

#include <netdb.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

/* buffered socket flag */
#define BS_F_SSL        0x01  /* Use SSL/TLS */
#define BS_F_SVR        0x02  /* Server Mode */
#define BS_F_CTX        0x04  /* Create SSL/TLS Context */
#define BS_F_KEP        0x08  /* Session Cache */

#define BS_RC_NEED_MORE        0
#define BS_RC_FINISHED        -5



#define BS_SUPP_LIMIT         1
#define BSOCK_PKG_FINISH      -5

#define INET_PORT_LEN         8
struct ev_buff {
	char *data, *start, *tail, *end;
	int size;
};

#define buff_left(buf)     ((buf)->end - (buf)->tail)
#define buff_length(buf)   ((buf)->tail - (buf)->start)
#define buff_restart(buf)  (buf)->tail = (buf)->start;
#define buff_string(buf)   ((buf)->start)
#define buff_seal(buf) do {             \
	if ((buf)->end == (buf)->tail)  \
		*((buf)->tail - 1) = 0; \
	else                            \
		*(buf)->tail = 0;       \
} while(0)
int buff_drain(struct ev_buff *buff, int len);

enum ev_bsock_states {
	BS_S_INIT,
	BS_S_LISTENING,
	BS_S_SSL_HANDSHAKE,
	BS_S_CONNECTING,
	BS_S_CONNECTED,
	BS_S_FAILED,
	BS_S_SHUTDOWN,
	BS_S_CLOSED,
	BS_S_MAX
};

enum {
	BS_EV_MIN,
	BS_EV_TIMER,
	BS_EV_SSL_END,     /* SSL/TLS handshake finished */
	BS_EV_CONNECT,     /* connected succesfully */
	BS_EV_EOF,         /* read end of file */
	BS_EV_FINISH,      /* finish package process */
	BS_EV_MAX,

	BS_ERR_SOCKET,     /* create socket failed */
	BS_ERR_CONNECT,    /* connect failed */
	BS_ERR_CONN_TO,    /* connect time out */
	BS_ERR_WRITE,      /* write failed */
	BS_ERR_READ,       /* read failed */
	BS_ERR_NOBUFF,     /* not enough read buffer */
	BS_ERR_PACKAGE,    /* parse package failed */
	BS_ERR_MAX,
};

struct bsock_cert {
        const char *cacert;
        const char *key;
        const char *cert;
};

struct ev_bstls {
	SSL *ssl;
	SSL_CTX *ctx;
};

struct ev_bsock {
	int flag;
	int state, fd, errcode;

	struct socketinfo addr;
	char ipaddr[INET6_ADDRSTRLEN];
	char ipport[INET_PORT_LEN];   /* max 65536 */

	struct ev_io rsock, wsock;      /* read/write socket */
	struct ev_buff *wbuf, *rbuf;    /* read/write buffer */
	struct ev_timer rtimer;         /* read & connection timeoout timer */
	struct ev_timer wtimer;         /* read/write timer  */

	int connect_timeout;

	struct ev_bstls *tls;

#if BS_SUPP_LIMIT == 1
	double delay;
	struct ev_timer ltimer;         /* limiter timer */
#endif

	int (*package)(char *data, int len, void *arg);
	void (*event)(struct ev_bsock *bs, int event, void *arg);

	void *arg;
};

#define bsock_set_arg(bs, arg_)            (bs)->arg = (arg_)
#define bsock_set_read(bs, cb)             (bs)->read = (cb)
#define bsock_set_package(bs, cb)          (bs)->package = (cb)
#define bsock_set_write(bs, cb)            (bs)->write = (cb)
#define bsock_set_event(bs, cb)            (bs)->event = (cb)
#define bsock_set_connect_timeout(bs, v)   (bs)->connect_timeout = (v)
#define bsock_get_timer(bs)                (bs)->wtimer

struct ev_bsock *bsock_create(int wbsize, int rbsize, int flag);
int bsock_listen(struct ev_bsock *bs);
int bsock_connect(struct ev_bsock *bs);

int bsock_set_sock(struct ev_bsock *bs, struct addrinfo *addr);
int bsock_set_socket(struct ev_bsock *bs, struct socketinfo *sock);

int bsock_write(struct ev_bsock *bs, const char *data, int len);

const char *bsock_get_event(int event);
int bsock_is_error_event(int event);



/* SSL/TLS related stuff */
int bsock_ssl_init(void);
SSL_CTX *bsock_ssl_create_context(int flag);
SSL *bsock_ssl_create(SSL_CTX *ctx, int sockfd, int flag);

void bsock_tls_showinfo(struct ev_bsock *bs, SSL *ssl);
int bsock_tls_verify_certs(struct ev_bsock *bs, SSL *ssl);
int bsock_tls_setup_cert(struct ev_bsock *bs, struct bsock_cert *cert);

void bsock_ssl_handshake(EV_P_ struct ev_bsock *bs);
int bsock_tls_read(struct ev_bsock *bs, char *buff, int len);
int bsock_tls_write(struct ev_bsock *bs, char *buff, int len);

void bsock_tls_shutdown(SSL *ssl);


/* stop all event, reset socket buffer */
void bsock_reset(EV_P_ struct ev_bsock *bs);

/* reset and shutdown SSL if needed */
void bsock_shutdown(EV_P_ struct ev_bsock *bs, int flag);

/* reset and close socket */
void bsock_close(EV_P_ struct ev_bsock *bs, int flag);

/* reset close and destroy all */
void bsock_destroy(EV_P_ struct ev_bsock *bs, int flag);

#endif
