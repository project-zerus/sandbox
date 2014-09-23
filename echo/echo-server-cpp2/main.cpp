
#include <signal.h>

#include <thread>

#include <memory>

#include "boost/shared_ptr.hpp"
#include "boost/cstdint.hpp"

#include "thrift/lib/cpp/Thrift.h"
#include "thrift/lib/cpp/TProcessor.h"
#include "thrift/lib/cpp/async/TEventServer.h"
#include "thrift/lib/cpp/concurrency/PosixThreadFactory.h"
#include "thrift/lib/cpp/concurrency/ThreadManager.h"
#include "thrift/lib/cpp/protocol/TBinaryProtocol.h"
#include "thrift/lib/cpp/protocol/THeaderProtocol.h"
#include "thrift/lib/cpp/protocol/TProtocol.h"
#include "thrift/lib/cpp/transport/TBufferTransports.h"
#include "thrift/lib/cpp/transport/TServerTransport.h"
#include "thrift/lib/cpp/transport/TServerSocket.h"
#include "thrift/lib/cpp/util/TEventServerCreator.h"

#include "EchoHandler.h"

using apache::thrift::async::TAsyncProcessor;
using apache::thrift::async::TEventServer;
using apache::thrift::util::TEventServerCreator;

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::server;
using namespace apache::thrift::transport;

using namespace sandbox::echo::thrift;
using namespace sandbox::echo::server;

void sig_handler(int s){
  std::cout << "Caught signal " << s << std::endl;
  exit(1);
}


int main(int argc, char** argv) {
  signal(SIGINT,  sig_handler);
  signal(SIGABRT, sig_handler);
  signal(SIGQUIT, sig_handler);
  signal(SIGTERM, sig_handler);

  auto n = std::thread::hardware_concurrency();
  std::cout << "Number of cores: " << n << std::endl;

  std::shared_ptr<EchoHandler> handler(new EchoHandler());
  std::shared_ptr<TProcessor> processor(new EchoProcessor(handler));

  TEventServerCreator serverCreator(processor, 20000);
  std::shared_ptr<TEventServer> server = serverCreator.createEventServer();
  server->serve();

  return 0;
}
