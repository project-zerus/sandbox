
#include "apns.h"

SSL_CTX*
apns_ssl_init(const std::string& pemPath)
{
    SSL_CTX* client_ctx;

    /* Initialize the OpenSSL library */
    SSL_load_error_strings();
    SSL_library_init();

    /* We MUST have entropy, or else there's no point to crypto. */
    if (!RAND_poll())
        return NULL;

    client_ctx = SSL_CTX_new(SSLv23_client_method());

    if (SSL_CTX_use_PrivateKey_file(client_ctx, pemPath.c_str(), SSL_FILETYPE_PEM)) {
        return NULL;
    }

    SSL_CTX_set_options(client_ctx, SSL_OP_NO_SSLv2);

    return client_ctx;
}

bufferevent*
apns_bufferevent_init(event_base* base, SSL* ssl, const std::string& host, const int port) {
    return NULL;
}
