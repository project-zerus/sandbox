#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "essence/libevhtp/evhtp.h"

#include "apns.h"

struct app_parent {
    evhtp_t*    evhtp;
    evbase_t*   evbase;
    const char* apns_host;
    uint16_t    apns_port;
};

struct app {
    app_parent*  parent;
    evbase_t*    evbase;

    SSL_CTX*     apnsSslCtx;
    bufferevent* apnsBufferEvent;
};

static evthr_t*
get_request_thr(evhtp_request_t* request) {
    evhtp_connection_t* htpconn;
    evthr_t*            thread;

    htpconn = evhtp_request_get_connection(request);
    thread  = htpconn->thread;

    return thread;
}

void
app_process_request(evhtp_request_t* request, void* arg) {
  const char* str = "pong\n";
  evbuffer_add_printf(request->buffer_out, "%s", str);
  evhtp_send_reply(request, EVHTP_RES_OK);
}

void
app_init_thread(evhtp_t* htp, evthr_t* thread, void* arg) {
    struct app_parent* app_parent;
    struct app*        app;

    app_parent  = (struct app_parent*)arg;
    app         = (struct app*)calloc(sizeof(struct app), 1);

    app->parent = app_parent;
    app->evbase = evthr_get_base(thread);

    app->apnsSslCtx = apns_ssl_init("cert.pem");
    assert(app->apnsSslCtx);

    //bufferevent_openssl_socket_new(app->evbase, )
//    app->redis  = redisAsyncConnect(app_parent->redis_host, app_parent->redis_port);
//    redisLibeventAttach(app->redis, app->evbase);

    evthr_set_aux(thread, app);
}

int
main(int argc, char** argv) {
    evbase_t*          evbase;
    evhtp_t*           evhtp;
    struct app_parent* app_p;

    evbase            = event_base_new();
    evhtp             = evhtp_new(evbase, NULL);
    app_p             = (app_parent*)calloc(sizeof(struct app_parent), 1);

    app_p->evhtp      = evhtp;
    app_p->evbase     = evbase;
    app_p->apns_host = "gateway.sandbox.push.apple.com";
    app_p->apns_port = 2195;

    evhtp_set_gencb(evhtp, app_process_request, NULL);
    evhtp_use_threads(evhtp, app_init_thread, 8, app_p);
    evhtp_bind_socket(evhtp, "127.0.0.1", 9000, 32768);

    event_base_loop(evbase, 0);

    return 0;
}

