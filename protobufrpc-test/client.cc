
#include <stdio.h>

#include <boost/bind.hpp>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/protorpc/RpcChannel.h>


#include "toft/system/time/clock.h"

#include "sandbox/protobufrpc-test/echo.pb.h"

using namespace muduo;
using namespace muduo::net;

const int N = 65536;

class RpcClient : boost::noncopyable
{
 public:
  RpcClient(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      client_(loop, serverAddr, "RpcClient"),
      channel_(new RpcChannel),
      stub_(get_pointer(channel_)),
      echoReceived_(0),
      beginTime_(0),
      endTime_(0)
  {
    client_.setConnectionCallback(
        boost::bind(&RpcClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&RpcChannel::onMessage, get_pointer(channel_), _1, _2, _3));
    // client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      channel_->setConnection(conn);
      beginTime_ = toft::RealtimeClock.MilliSeconds();
      for (int i = 0; i < N; ++i) {
        echo::EchoRequest request;
        request.set_text("hi");
        echo::EchoResponse* response = new echo::EchoResponse;
        stub_.echo(NULL, &request, response, NewCallback(this, &RpcClient::onEchoResponse, response));
      }
    }
  }

  void onEchoResponse(echo::EchoResponse* resp)
  {
    //LOG_INFO << "onEchoResponse:\n" << resp->DebugString().c_str();
    ++echoReceived_;
    if (N == echoReceived_)
    {
      endTime_ = toft::RealtimeClock.MilliSeconds();
      double qps = ((double)N) * 1000.0 / (double)(endTime_ - beginTime_);
      LOG_INFO
        << "begin time: " << beginTime_ << "; "
        << "end time: " << endTime_ << "; "
        << "qps: " << qps;
      loop_->quit();
    }
  }

  EventLoop* loop_;
  TcpClient client_;
  RpcChannelPtr channel_;
  echo::EchoService::Stub stub_;
  int echoReceived_;
  int64_t beginTime_;
  int64_t endTime_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress serverAddr("127.0.0.1", 9981);
  RpcClient rpcClient(&loop, serverAddr);
  rpcClient.connect();
  loop.loop();
  google::protobuf::ShutdownProtobufLibrary();
}

