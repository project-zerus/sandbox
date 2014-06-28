
#include "sandbox/echo/echo-thrift-api/gen-cpp2/Echo.h"

#include <iostream>
#include <memory>
#include <utility>

#include "thrift/lib/cpp/Thrift.h"
#include "thrift/lib/cpp/async/TAsyncSocket.h"
#include "thrift/lib/cpp/async/TAsyncTransport.h"
#include "thrift/lib/cpp/async/TEventBase.h"
#include "thrift/lib/cpp/async/TFramedAsyncChannel.h"
#include "thrift/lib/cpp/protocol/TBinaryProtocol.h"
#include "thrift/lib/cpp/protocol/TProtocol.h"
#include "thrift/lib/cpp/transport/TSocket.h"
#include "thrift/lib/cpp/transport/TBufferTransports.h"

#include "thrift/lib/cpp2/async/Cpp2Channel.h"
#include "thrift/lib/cpp2/async/DuplexChannel.h"
#include "thrift/lib/cpp2/async/HeaderClientChannel.h"
#include "thrift/lib/cpp2/async/RequestChannel.h"

int
main(int argc, char** argv) {
  using apache::thrift::Cpp2Channel;
  using apache::thrift::DuplexChannel;
  using apache::thrift::HeaderClientChannel;
  using apache::thrift::RequestChannel;
  using apache::thrift::TException;
  using apache::thrift::async::TAsyncSocket;
  using apache::thrift::async::TEventBase;
  using apache::thrift::async::TFramedAsyncChannel;
  using apache::thrift::protocol::TBinaryProtocol;
  using apache::thrift::transport::TSocket;
  using apache::thrift::transport::TFramedTransport;
  using sandbox::echo::thrift::cpp2::EchoAsyncClient;

  TEventBase base;
  std::shared_ptr<TAsyncSocket> socket(
    TAsyncSocket::newSocket(&base, "127.0.0.1", 20000)
  );
  auto clientChannel = HeaderClientChannel::newChannel(socket);
  EchoAsyncClient client(std::move(clientChannel));
  auto future = client.future_echo("hello");
  base.loop();
  std::cout << future.value() << std::endl;
  return 0;
}
