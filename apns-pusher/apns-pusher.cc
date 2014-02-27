#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "openssl/ssl.h"
#include "openssl/rand.h"
#include "openssl/bio.h"
#include "openssl/err.h"
#include "openssl/x509.h"

SSL*     ssl;
SSL_CTX* ctx;

void error(const char *msg)
{
    printf("[ERROR]:%s\n", msg);
    exit(1);
}

SSL_CTX* setup_client_ctx()
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

void token2bytes(const char* token, char* bytes)
{
    int val;
    while (*token) {
        sscanf(token, "%2x", &val);
        *(bytes++) = (char)val;
        token += 2;
        while (*token == ' ') { // skip space
            ++token;
        }
    }
}

unsigned long packMessage(char* message,
                          const unsigned char command,
                          const char* tokenBytes,
                          const char* payload)
{
    unsigned long payloadLength = strlen(payload);
    unsigned short networkTokenLength = htons(32);
    unsigned short networkPayloadLength = htons(payloadLength);
    memcpy(message, &command, sizeof(unsigned char));
    message += sizeof(unsigned char);
    memcpy(message, &networkTokenLength, sizeof(unsigned short));
    message += sizeof(unsigned short);
    memcpy(message, tokenBytes, 32);
    message += 32;
    memcpy(message, &networkPayloadLength, sizeof(unsigned short));
    message += sizeof(unsigned short);
    memcpy(message, payload, payloadLength);
    return payloadLength + 37;
}

int push(const char* token, const char* payload)
{
    char tokenBytes[32];
    char message[293];

    unsigned long msgLength;

    token2bytes(token, tokenBytes);

    msgLength = packMessage(message, 0, tokenBytes, payload);

    return SSL_write(ssl, message, (int)msgLength);
}

int main (int argc, const char* argv[])
{
    char token[] = "6fdb70e7 0cea3ce7 61dd8f0f 0504cb52 28b3cedb 16fa25c4 da2ac265 9c886862";
    char payload[] = "{\"aps\":{\"alert\":\"Hello world!!!\",\"badge\":1}}";

    // http://pubs.opengroup.org/onlinepubs/009695399/functions/getaddrinfo.html
    // use getaddrinfo() to do dns resolution
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

    int ret = push(token, payload);
    printf("push ret %d\n", ret);

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return 0;
}
