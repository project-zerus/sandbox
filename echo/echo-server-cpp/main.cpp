
#include <signal.h>

#include <thread>

#include "thirdparty/boost/shared_ptr.hpp"
#include "thirdparty/boost/cstdint.hpp"

#include "thrift/Thrift.h"
#include "thrift/TProcessor.h"
#include "thrift/concurrency/PosixThreadFactory.h"
#include "thrift/concurrency/ThreadManager.h"
#include "thrift/protocol/TBinaryProtocol.h"
#include "thrift/protocol/TProtocol.h"
#include "thrift/server/TNonblockingServer.h"
#include "thrift/transport/TBufferTransports.h"
#include "thrift/transport/TServerTransport.h"
#include "thrift/transport/TServerSocket.h"

#include "EchoHandler.h"

using boost::shared_ptr;

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

  shared_ptr<EchoHandler> handler(new EchoHandler());
  shared_ptr<TProcessor> processor(new EchoProcessor(handler));
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(n);
  shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
  threadManager->threadFactory(threadFactory);
  threadManager->start();

  TNonblockingServer server(processor, protocolFactory, 20000, threadManager);
  server.serve();

  return 0;
}
