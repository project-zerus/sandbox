#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "openssl/ssl.h"
#include "openssl/rand.h"
#include "openssl/bio.h"
#include "openssl/err.h"
#include "openssl/x509.h"

SSL *ssl;
SSL_CTX *ctx;

void error(const char *msg)
{
    printf("[ERROR]:%s\n", msg);
    exit(1);
}

SSL_CTX *setup_client_ctx()
{
    ctx = SSL_CTX_new(SSLv23_method());
    if (SSL_CTX_use_certificate_chain_file(ctx, "cert.pem") != 1) {
        error("Error loading certificate from file\n");
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "cert.pem", SSL_FILETYPE_PEM) != 1) {
        error("Error loading private key from file\n");
    }
    return ctx;
}

int main (int argc, const char* argv[])
{
//    char token[] = "2b2474e5 ac7670f3 08fabf3a 9c1d1295 ed50e9aa f11b941a d6e3d213 4f535408";
//    char payload[] = "{\"aps\":{\"alert\":\"Hello world!!!\",\"badge\":1}}";
//    char payload2[] = "{\"aps\":{\"alert\":\"Hello kitty!!!\",\"badge\":12}}";

    char host[] = "gateway.sandbox.push.apple.com:2195";

    BIO* conn;

    // init
    SSL_library_init();
    ctx = setup_client_ctx();
    conn = BIO_new_connect(host);

    if (!conn) {
        error("Error creating connection BIO\n");
    }

    if (BIO_do_connect(conn) <= 0) {
        error("Error connection to remote machine");
    }

    if (!(ssl = SSL_new(ctx))) {
        error("Error creating an SSL contexxt");
    }

    SSL_set_bio(ssl, conn, conn);

    if (SSL_connect(ssl) <= 0) {
        error("Error connecting SSL object");
    }

    printf("SSL Connection opened\n");

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return 0;
}
