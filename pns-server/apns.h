#ifndef _APNS_H_
#define _APNS_H_

#include <string>

#include "thirdparty/openssl/ssl.h"
#include "thirdparty/openssl/err.h"
#include "thirdparty/openssl/rand.h"

#include "thirdparty/libevent/event2/bufferevent_ssl.h"

SSL_CTX*
apns_ssl_init(const std::string& pemPath);

#endif // _APNS_H_
