#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXBUF 1023

int verify_show_certs(SSL *ssl)
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

int main(void)
{
        SSL *ssl;
        SSL_CTX *ctx;
        socklen_t len;
        char buff[MAXBUF + 1];
        int sockfd, clifd, rc;
        struct sockaddr_in svraddr, cliaddr;

	signal(SIGPIPE, SIG_IGN);

        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        ctx = SSL_CTX_new(SSLv23_server_method()); /* support both V2 and V3 */
        if (ctx == NULL) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }

        /* set our supported ciphers */
        if(SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH") != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }

        /* the client have to send it's certificate, default SSL_VERIFY_NONE */
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

        /* load ca file */
        if (SSL_CTX_load_verify_locations(ctx, "pki/CA/cacert.pem", 0) != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }

#if 0
        /* enable srtp */
        r = SSL_CTX_set_tlsext_use_srtp(k->ctx, "SRTP_AES128_CM_SHA1_80");
        if(r != 0) {
                printf("Error: cannot setup srtp.\n");
                ERR_print_errors_fp(stderr);
                return -3;
        }
#endif
        /* load certificate file, which will send public key to client. */
        if (SSL_CTX_use_certificate_file(ctx, "pki/SVR/cert.pem", SSL_FILETYPE_PEM) != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
        /* and also private key file */
        if (SSL_CTX_use_PrivateKey_file(ctx, "pki/SVR/key.pem", SSL_FILETYPE_PEM) != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
        /* check if the private key is valid */
        if (SSL_CTX_check_private_key(ctx) != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
        SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

        sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
                fprintf(stderr, "create socket failed, %s.\n", strerror(errno));
                exit(1);
        }

        memset(&svraddr, 0, sizeof(svraddr));
        svraddr.sin_family = PF_INET;
        svraddr.sin_port = htons(8090);
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

                if (verify_show_certs(ssl) < 0) {
                        fprintf(stderr, "invalid certs from peer.\n");
			SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
                        //SSL_shutdown(ssl);
                        SSL_free(ssl);
                        close(clifd);
                        continue;
                }

                rc = SSL_read(ssl, buff, sizeof(buff) - 1); /* for '\0' */
                if (rc < 0) {
                        fprintf(stderr, "read from SSL failed.\n");
			SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
                        //SSL_shutdown(ssl);
                        SSL_free(ssl);
                        close(clifd);
                        continue;
                }
                buff[rc] = 0;
                fprintf(stdout, "got data(%02d): %s\n", rc, buff);

                rc = SSL_write(ssl, buff, rc); /* for '\0' */


		SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
                //SSL_shutdown(ssl);
                SSL_free(ssl);
                close(clifd);
        }

        SSL_CTX_free(ctx);
        return 0;
}
