/**
 * @author Huahang Liu (huahang@xiaomi.com)
 * @date 2014-03-21
 */
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

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

#include "sandbox/echo/echo-thrift-api/Echo.h"

void test() {
  using boost::shared_ptr;
  using namespace sandbox::echo::thrift;
  using namespace apache::thrift;
  using namespace apache::thrift::concurrency;
  using namespace apache::thrift::protocol;
  using namespace apache::thrift::server;
  using namespace apache::thrift::transport;
  shared_ptr<TSocket> socket(new TSocket("localhost", 20000));
  shared_ptr<TFramedTransport> transport(new TFramedTransport(socket));
  shared_ptr<TBinaryProtocol> protocol(new TBinaryProtocol(transport));
  EchoClient client(protocol);
  try {
    std::cout << "before transport->open()" << std::endl;
    transport->open();
    std::cout << "after transport->open()" << std::endl;
    std::string echoMessage;
    client.echo(echoMessage, "hi");
    std::cout << "echoMessage " << echoMessage << std::endl;
    transport->close();
  } catch (TException &tx) {
    std::cout << "ERROR: " << tx.what() << std::endl;
  }
}

int main(int argc, char** argv) {
  std::thread testThread(test);
  testThread.join();
  return 0;
}
