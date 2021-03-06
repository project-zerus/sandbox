
#include <unistd.h>
#include <iostream>

#include <sofa/pbrpc/pbrpc.h>

#include "glog/logging.h"

#include "sandbox/sofa/echo/echo_service.pb.h"

void EchoCallback(
  sofa::pbrpc::RpcController* cntl,
  sofa::pbrpc::test::EchoRequest* request,
  sofa::pbrpc::test::EchoResponse* response,
  bool* callbacked) {
  SLOG(NOTICE, "RemoteAddress=%s", cntl->RemoteAddress().c_str());
  SLOG(NOTICE, "IsRequestSent=%s", cntl->IsRequestSent() ? "true" : "false");
  if (cntl->IsRequestSent()) {
    SLOG(NOTICE, "LocalAddress=%s", cntl->LocalAddress().c_str());
    SLOG(NOTICE, "SentBytes=%ld", cntl->SentBytes());
  }

  if (cntl->Failed()) {
    SLOG(ERROR, "request failed: %s", cntl->ErrorText().c_str());
  }
  else {
    std::cout << "request succeed: " << response->message() << std::endl;
  }

  delete cntl;
  delete request;
  delete response;
  *callbacked = true;
}

int main()
{
  SOFA_PBRPC_SET_LOG_LEVEL(WARNING);

  // Define an rpc server.
  sofa::pbrpc::RpcClientOptions client_options;
  sofa::pbrpc::RpcClient rpc_client(client_options);

  // Define an rpc channel.
  sofa::pbrpc::RpcChannelOptions channel_options;
  sofa::pbrpc::RpcChannel rpc_channel(
    &rpc_client,
    "127.0.0.1:12321",
    channel_options
  );

  // Prepare parameters.
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);
  sofa::pbrpc::test::EchoRequest* request =
    new sofa::pbrpc::test::EchoRequest();
  request->set_message("你好");
  sofa::pbrpc::test::EchoResponse* response =
    new sofa::pbrpc::test::EchoResponse();
  bool callbacked = false;
  google::protobuf::Closure* done = sofa::pbrpc::NewClosure(
    &EchoCallback, cntl, request, response, &callbacked
  );

  // Async call.
  sofa::pbrpc::test::EchoServer_Stub stub(&rpc_channel);
  stub.Echo(cntl, request, response, done);

  // Wait call done.
  while (!callbacked) {
    usleep(100000);
  }

  return EXIT_SUCCESS;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
