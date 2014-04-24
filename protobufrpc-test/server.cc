#include "sandbox/protobufrpc-test/echo.pb.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/protorpc/RpcServer.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

namespace echo
{

class EchoServiceImpl : public EchoService
{
 public:
  virtual void echo(
    ::google::protobuf::RpcController* controller,
    const ::echo::EchoRequest* request,
    ::echo::EchoResponse* response,
    ::google::protobuf::Closure* done)
  {
    response->set_text(request->text());
    done->Run();
  }
};

}

int main()
{
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listenAddr(9981);
  echo::EchoServiceImpl impl;
  RpcServer server(&loop, listenAddr);
  server.registerService(&impl);
  server.start();
  loop.loop();
  google::protobuf::ShutdownProtobufLibrary();
}

