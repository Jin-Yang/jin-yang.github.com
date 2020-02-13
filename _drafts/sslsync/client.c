#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

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

#if 0
        //ECDHE-ECDSA-AES256-GCM-SHA384:RC4:HIGH:!MD5:!aNULL:!EDH
        /* set our supported ciphers */
        if (SSL_CTX_set_cipher_list(ctx, "AES256-GCM-SHA384") != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
#endif
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

        /* load ca file */
        if (SSL_CTX_load_verify_locations(ctx, "pki/CA/cacert.pem", 0) != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
        if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }

        /* load certificate file, which will send public key to client. */
        if (SSL_CTX_use_certificate_file(ctx, "pki/CLI/cert.pem", SSL_FILETYPE_PEM) != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
        /* and also private key file */
        if (SSL_CTX_use_PrivateKey_file(ctx, "pki/CLI/key.pem", SSL_FILETYPE_PEM) != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
        /* check if the private key is valid */
        if (SSL_CTX_check_private_key(ctx) != 1) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }
        SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
                fprintf(stderr, "create socket failed, %s.\n", strerror(errno));
                exit(1);
        }

        memset(&dst, 0, sizeof(dst));
        dst.sin_family = PF_INET;
        dst.sin_port = htons(8899);
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

        if (verify_show_certs(ssl) < 0) {
		fprintf(stderr, "invalid certs from peer.\n");
                SSL_shutdown(ssl);
                SSL_free(ssl);
                close(sockfd);
                SSL_CTX_free(ctx);
                return -1;
        }

        SSL_write(ssl, "Hello!", strlen("Hello!"));

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);

        return 0;
}
