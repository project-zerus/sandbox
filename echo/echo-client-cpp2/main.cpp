
#include "sandbox/echo/echo-thrift-api/gen-cpp2/Echo.h"

#include <iostream>
#include <memory>
#include <utility>

#include "folly/io/async/DelayedDestruction.h"

#include "thrift/lib/cpp/Thrift.h"
#include "thrift/lib/cpp/async/TAsyncSocket.h"
#include "thrift/lib/cpp/async/TAsyncTransport.h"
#include "thrift/lib/cpp/async/TDelayedDestruction.h"
#include "thrift/lib/cpp/async/TEventBase.h"
#include "thrift/lib/cpp/async/TFramedAsyncChannel.h"
#include "thrift/lib/cpp/protocol/TBinaryProtocol.h"
#include "thrift/lib/cpp/protocol/TProtocol.h"
#include "thrift/lib/cpp/transport/TSocket.h"
#include "thrift/lib/cpp/transport/TBufferTransports.h"

#include "thrift/lib/cpp2/async/DuplexChannel.h"
#include "thrift/lib/cpp2/async/HeaderClientChannel.h"

int
main(int argc, char** argv) {
  using apache::thrift::DuplexChannel;
  using apache::thrift::HeaderClientChannel;
  using apache::thrift::TException;
  using apache::thrift::async::TAsyncSocket;
  using apache::thrift::async::TEventBase;
  using apache::thrift::async::TFramedAsyncChannel;
  using apache::thrift::protocol::TBinaryProtocol;
  using apache::thrift::transport::TSocket;
  using apache::thrift::transport::TFramedTransport;
  using folly::DelayedDestruction;
  using sandbox::echo::thrift::cpp2::EchoAsyncClient;

  TEventBase base;
  std::shared_ptr<TAsyncSocket> socket(
    TAsyncSocket::newSocket(&base, "127.0.0.1", 20000)
  );
  std::unique_ptr<
    HeaderClientChannel,
    DelayedDestruction::Destructor
  > headerChannel(
    HeaderClientChannel::newChannel(socket)
  );
  headerChannel->getHeader()->setProtocolId(
    apache::thrift::protocol::T_BINARY_PROTOCOL
  );
  headerChannel->getHeader()->setClientType(THRIFT_FRAMED_DEPRECATED);
  EchoAsyncClient client(std::move(headerChannel));
  auto future = client.future_echo("hello");
  base.loop();
  std::cout << future.value() << std::endl;
  return 0;
}
