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

这里采用子签发的证书，需要生成根证书，并分别向两端办法对应的证书。

### 生成证书

{% highlight text %}
mkdir pki/{CA,SVR,CLI} -p
{% endhighlight %}

子签CA证书。

{% highlight text %}
----- 生成根证书私钥 pem
openssl genrsa -out cakey.pem 2048
----- 生成根证书签发申请文件 csr
openssl req -new -key cakey.pem -out ca.csr    \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyCA"
----- 自签发根证书 cer
openssl x509 -req -days 3650 -sha1 -extensions v3_ca -signkey cakey.pem -in ca.csr -out cacert.pem
{% endhighlight %}

服务端私钥和证书。

{% highlight text %}
----- 生成服务端私钥
openssl genrsa -out key.pem 2048
----- 生成证书请求文件
openssl req -new -key key.pem -out server.csr  \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyServer"
----- 使用根证书签发服务端证书
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA ../CA/cacert.pem     \
	-CAkey ../CA/cakey.pem -CAserial ca.srl -CAcreateserial -in server.csr -out cert.pem
----- 使用CA证书验证server端证书
openssl verify -CAfile ../CA/cacert.pem cert.pem
{% endhighlight %}


客户端私钥和证书。

{% highlight text %}
----- 生成客户端私钥
openssl genrsa -out key.pem 2048
----- 生成证书请求文件
openssl req -new -key key.pem -out client.csr  \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyClient"
----- 使用根证书签发客户端证书
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA ../CA/cacert.pem     \
	-CAkey ../CA/cakey.pem -CAserial ../SVR/ca.srl -in client.csr -out cert.pem
----- 使用CA证书验证客户端证书
openssl verify -CAfile ../CA/cacert.pem cert.pem
{% endhighlight %}

也可以对私钥进行加密。

{% highlight text %}
----- 通过AES256解密保护私钥
openssl genrsa -aes256 -out keysec.pem 2048
openssl req -new -key keysec.pem -out clientsec.csr   \
	-subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyClient"
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA ../CA/cacert.pem     \
	-CAkey ../CA/cakey.pem -CAserial ../SVR/ca.srl -in clientsec.csr -out certsec.pem

----- 去除私钥中的密码保护
openssl rsa -in pki/CLI/keysec.pem -out pki/CLI/keyplain.pem
{% endhighlight %}

其它。

{% highlight text %}
----- 查看证书的内容
openssl x509 -in SVR/cert.pem -text -noout
{% endhighlight %}

<!--
https://ningyu1.github.io/site/post/51-ssl-cert/
吊销证书：$ openssl ca -revoke cert.pem -config openssl.cnf
证书吊销列表：$ openssl ca -gencrl -out cacert.crl -config openssl.cnf
查看列表内容：$ openssl crl -in cacert.crl -text -noout
-->

### 测试

OpenSSL 提供了 Server 和 Client 的相关工具，可以用来进行测试，如下是常用的参数。

{% highlight text %}
-connect    指定服务器的地址以及端口，默认是localhost:443
-key        私钥文件的路径
-cert       证书文件的路径
-CAfile     根证书文件的路径
-showcerts  显示服务器的证书信息
-state      在SSL交互过程中的各种信息
-verify     根证书校验的深度
-debug      打印调试信息
-accept     监听的端口号
{% endhighlight %}

{% highlight text %}
openssl s_server -accept 44330 -CAfile pki/CA/cacert.pem            \
	-key pki/SVR/key.pem -cert pki/SVR/cert.pem -state
openssl s_client -connect 127.0.0.1:44330 -CAfile pki/CA/cacert.pem \
	-key pki/CLI/key.pem -cert pki/CLI/cert.pem -state
{% endhighlight %}



### 同步方式

<!--
http://wzhnsc.blogspot.com/2012/12/openssl-api.html
-->

{% highlight text %}
{% endhighlight %}
