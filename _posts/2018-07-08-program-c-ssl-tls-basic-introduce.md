---
title: C 语言使用 SSL
layout: post
comments: true
language: chinese
category: [program,linux]
keywords: openssl,ssl,c
description:
---

<!-- more -->



{% highlight text %}
$ openssl genrsa -out privkey.pem 2048
$ openssl req -new -x509 -key privkey.pem -out cacert.pem -days 1095
{% endhighlight %}

{% highlight c %}
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXBUF 1024

int main(void)
{
        SSL *ssl;
        SSL_CTX *ctx;
        socklen_t len;
        char buff[MAXBUF + 1];
        int sockfd, clifd, rc;
        struct sockaddr_in svraddr, cliaddr;

        SSL_library_init();
        //OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        ctx = SSL_CTX_new(SSLv23_server_method()); /* support both V2 and V3 */
        if (ctx == NULL) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }

        /* load certificate file, which will send public key to client. */
        if (SSL_CTX_use_certificate_file(ctx, "cacert.pem", SSL_FILETYPE_PEM) <= 0) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
        /* and also private key file */
        if (SSL_CTX_use_PrivateKey_file(ctx, "privkey.pem", SSL_FILETYPE_PEM) <= 0) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }

        sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
                fprintf(stderr, "create socket failed, %s.\n", strerror(errno));
                exit(1);
        }

        memset(&svraddr, 0, sizeof(svraddr));
        svraddr.sin_family = PF_INET;
        svraddr.sin_port = htons(8080);
        svraddr.sin_addr.s_addr = INADDR_ANY;
        if (bind(sockfd, (struct sockaddr *)&svraddr, sizeof(struct sockaddr)) < 0) {
                fprintf(stderr, "bind socket failed, %s.\n", strerror(errno));
                close(sockfd);
                exit(1);
        }

        if (listen(sockfd, 128) < 0) {
                fprintf(stderr, "listen socket failed, %s.\n", strerror(errno));
                close(sockfd);
                exit(1);
        }

        len = sizeof(struct sockaddr);
        while (1) {
                clifd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
                if (clifd < 0) {
                        fprintf(stderr, "listen socket failed, %s.\n", strerror(errno));
                        continue;
                }
                fprintf(stdout, "got connection #%d from [%s:%d]\n", clifd,
                        inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

                ssl = SSL_new(ctx);
                if (ssl == NULL) {
                        fprintf(stderr, "create SSL context failed.\n");
                        close(clifd);
                        continue;
                }
                SSL_set_fd(ssl, clifd);

                if (SSL_accept(ssl) < 0) {
                        fprintf(stderr, "accept SSL failed.\n");
                        close(clifd);
                        continue;
                }

                rc = SSL_read(ssl, buff, sizeof(buff) - 1); /* for '\0' */
                if (rc < 0) {
                        fprintf(stderr, "read from SSL failed.\n");
                        close(clifd);
                        continue;
                }
                buff[rc] = 0;
                fprintf(stdout, "got data(%2d): %s\n", rc, buff);

                SSL_shutdown(ssl);
                SSL_free(ssl);
                close(clifd);
        }

        SSL_CTX_free(ctx);
        return 0;
}
{% endhighlight %}

{% highlight c %}
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

void show_certs(SSL *ssl)
{
        X509 *cert;
        char *line;

        cert = SSL_get_peer_certificate(ssl);
        if (cert == NULL) {
                printf("No certificate information!\n");
                return;
        }
        printf("Digital certificate information:\n");

        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Certificate: %s\n", line);
        free(line);

        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);

        X509_free(cert);
}



int main(void)
{
        SSL *ssl;
        SSL_CTX *ctx;
        int sockfd, rc;
        struct sockaddr_in dst;

        SSL_library_init();
        //OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        ctx = SSL_CTX_new(SSLv23_client_method());
        if (ctx == NULL) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
                fprintf(stderr, "create socket failed, %s.\n", strerror(errno));
                exit(1);
        }

        memset(&dst, 0, sizeof(dst));
        dst.sin_family = PF_INET;
        dst.sin_port = htons(8080);
        inet_aton("127.0.0.1", (struct in_addr *)&dst.sin_addr.s_addr);
        rc = connect(sockfd, (struct sockaddr *)&dst, sizeof(dst));
        if (rc < 0) {
                fprintf(stderr, "connect to server failed, %s.\n", strerror(errno));
                close(sockfd);
                exit(1);
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sockfd);
        if (SSL_connect(ssl) < 0) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        show_certs(ssl);

        SSL_write(ssl, "Hello!", strlen("Hello!"));

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);

        return 0;
}
{% endhighlight %}

<!--
http://wzhnsc.blogspot.com/2012/12/openssl-api.html
-->

{% highlight text %}
{% endhighlight %}
