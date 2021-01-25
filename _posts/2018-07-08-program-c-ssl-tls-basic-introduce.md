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

## 无校验

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

## 双向认证




### 同步方式

在创建上下文时使用的是 `SSLv23_method()` 函数，这并不意味着只使用 TLSv1.2 以及 TLSv1.3 版本的协议，这是由于历史原因导致，实际的意思是指服务端和客户端协商使用双方都兼容最高版本，在 1.1.0 版本之后会修改为 `TLS_method()` 。

如果要屏蔽部分版本，可以通过 `SSL_CTX_set_options()` 函数屏蔽，例如参数 `SSL_OP_NO_SSLv2` `SSL_OP_NO_TLSv1_1` 等等。

<!--
http://wzhnsc.blogspot.com/2012/12/openssl-api.html
-->


## 会话复用

将上次通过握手计算出来的对称密钥复用，可以通过一次 RTT 完成握手。

{% highlight text %}
          <Client>                                          <Server>
       [Client Hello] -------------------------------->
                        * Supported Ciphers
                        * Random Number
                        * Session ID(**)
                        * SNI

                      <-------------------------------- [Server Hello]
                        * Session ID(reuse)             [Change Cipher Spec
                                                         Finished(encrypt)]
 [Change Cipher Spec
   Finished(encrypt)]

   [Application Data] <-------------------------------> [Application Data]
{% endhighlight %}

客户端在发送 `Client Hello` 请求的时候，会将上次握手过程中 Server 发送的 SessionID 带上，如果在服务端可以匹配到相关的信息，那么就直接返回成功，然后就可以交换应用数据。

SessionID 存在一些问题，如果服务端采用了分布式，当同一个客户端的请求没有落到上次的服务器时，会实效；服务端在使用 SessionID 时，很难判断其保存时间的长短。

### SessionTicket

而 Session Ticket 是在服务端加密之后的会话信息，然后保存在客户端中，下次请求会带上 Session Ticket ，只要服务端可以解密成功，那么就直接完成了握手。

可以使用 [github.com](https://github.com/vincentbernat/rfc5077) 中提供的工具测试支持情况。

OpenSSL 会将会话信息保存在上下文中，在 TLSv1.2 之前是在握手阶段发送会话 ID ，

## 其它

### 错误处理

大部分 OpenSSL 的函数在成功时返回 1 ，大部分失败返回 0 ，也有少量函数返回 -1 的，例如 `SSL_connect()` 函数，所以，可以通过 `!= 1` 判断是否异常。

详细的错误信息会保存在错误队列中 (一个 OpenSSL 实现的线程变量中)，可能会返回多个错误信息，可以通过函数 `ERR_print_errors_fp()` 或者 `ERR_print_errors()` 打印所有错误信息。

当有异常发生时，会将一些重要的信息记录下来，其中最基本的是 32bits 的错误码，如果无消息则会返回 0 ，常用的使用函数如下。

{% highlight text %}
/* 只获取错误代码 */
unsigned long ERR_get_error(void);       // 第一个，并将其移除队列
unsigned long ERR_peek_error(void);      // 第一个，不移除队列
unsigned long ERR_peek_last_error(void); // 最后一个，不移除队列

/* 获取错误码、错误产生的源文件、行号 */
unsigned long ERR_get_error_line(const char **file, int *line);
unsigned long ERR_peek_error_line(const char **file, int *line);
unsigned long ERR_peek_last_error_line(const char **file, int *line);

/* 除了获取上述信息外，还包括了额外的数据及如何处理这些数据的标示 */
unsigned long ERR_get_error_line_data(const char **file, int *line, const char **data, int *flags);
unsigned long ERR_peek_error_line_data(const char **file, int *line, const char **data, int *flags);
unsigned long ERR_peek_last_error_line_data(const char **file, int *line, const char **data, int *flags);

在通过最后一类获取数据时，多数是字符串，可以按照 C 的字符串方式使用；也可能是分配的内容，可以通过 `OPENSSL_free()` 函数释放。

/* 获取指定错误码的全部错误消息 */
char *ERR_error_string(unsigned long e, char *buf);
void  ERR_error_string_n(unsigned long e, char *buf, size_t len);

/* 获取指定错误码发生的库、函数、原因 */
const char *ERR_lib_error_string(unsigned long e);
const char *ERR_func_error_string(unsigned long e);
const char *ERR_reason_error_string(unsigned long e);
{% endhighlight %}

上述的 buf 最少为 120 字节，会返回一个 `error:[error code]:[library name]:[function name]:[reason string]` 格式的字符串。

### TLSv1.3

对于 TLSv1.3 版本之后，Session Ticket 会在握手成功之后发送，此时需要调用 `SSL_read()` 函数，对于 OpenSSL 来说才会完成 Session 信息的接收。

这同样也意味着，如果客户端检查证书时发现异常，如果此时直接强制关闭文件描述符，而再触发 `SSL_read()` 时就可能会发生 `EPIPE` 报错，而且关闭时如果没有很好处理，那么就可能会导致资源泄漏。

### 关闭

需要通过 `SSL_shutdown()` 函数关闭，详细可以查看 [man 3 SSL_shutdown](https://www.openssl.org/docs/ssl/SSL_shutdown.html) 帮助文档


可以强制关闭链接，但可能会导致部分资源的泄漏，所以最好的方式是 Two-Way Shutdown ，也就是客户端可能仍然需要通过 `SSL_read()` 读取报文，例如上述的未完全读取会话时。

<!--
https://www.openssl.org/docs/manmaster/man3/SSL_shutdown.html
-->

## 参考

* [SSL Programming Tutorial](http://h30266.www3.hpe.com/odl/axpos/opsys/vmsos84/BA554_90007/ch04s03.html) 比较简单清晰介绍各个流程，不过有点老。

<!--
会话复用
https://nachtimwald.com/2014/10/06/client-side-session-cache-in-openssl/

可以参考 Nginx 的实现
https://github.com/nginx/nginx/blob/master/src/http/modules/ngx_http_ssl_module.c src/event/ngx_event_openssl_stapling.c src/event/ngx_event_openssl.c Hitch 1.5.2 使用的是libev https://github.com/varnish/hitch
一个简单的HTTP服务端实现
https://github.com/criticalstack/libevhtp

可以实现中间人攻击
https://github.com/droe/sslsplit

一个异步TLS的封装
https://github.com/deleisha/evt-tls

Golang的编程以及一些参考连接
https://github.com/denji/golang-tls

OpenSSL中文手册之ERR库详解
https://blog.csdn.net/liao20081228/article/details/76718342

OpenSSL API 调用错误处理上的细节问题
https://segmentfault.com/a/1190000015326570
-->
