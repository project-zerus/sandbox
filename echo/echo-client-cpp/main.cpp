/**
 * @author Huahang Liu (huahang@xiaomi.com)
 * @date 2014-03-21
 */
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <stdint.h>
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

#include "toft/system/time/clock.h"

#include "sandbox/echo/echo-thrift-api/Echo.h"

using boost::shared_ptr;
using namespace sandbox::echo::thrift;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::server;
using namespace apache::thrift::transport;

const int T = 100;
const int N = 100;

void echo1(boost::shared_ptr<TFramedTransport> transport) {
  shared_ptr<TBinaryProtocol> protocol(new TBinaryProtocol(transport));
  EchoClient client(protocol);
  std::string echoMessage;
  client.echo(echoMessage, "hi");
  // std::cout << "echoMessage " << echoMessage << std::endl;
}

void echo2(boost::shared_ptr<TFramedTransport> transport) {
  shared_ptr<TBinaryProtocol> protocol(new TBinaryProtocol(transport));
  EchoClient client(protocol);
  std::string echoMessage;
  client.echo(echoMessage, "hi2");
  std::cout << "echoMessage " << echoMessage << std::endl; 
}

void test() {
  shared_ptr<TSocket> socket(new TSocket("localhost", 20000));
  shared_ptr<TFramedTransport> transport(new TFramedTransport(socket));
  try {
    transport->open();
    for (int i = 0; i < N; ++i) {
      echo1(transport);
    }
    transport->close();
  } catch (TException &tx) {
    std::cout << "ERROR: " << tx.what() << std::endl;
    exit(1);
  }
}

int main(int argc, char** argv) {
  int64_t beginTime = toft::RealtimeClock.MilliSeconds();
  std::vector<std::thread*> threads;
  for (int i = 0; i < T; ++i) {
    threads.push_back(new std::thread(test));
  }
  for (int i = 0; i < T; ++i) {
    threads[i]->join();
    delete threads[i];
  }
  threads.clear();
  int64_t endTime = toft::RealtimeClock.MilliSeconds();

  double qps = ((double)N * T) * 1000.0 / (double)(endTime - beginTime);
  std::cout
    << "begin time: " << beginTime << "; "
    << "end time: " << endTime << "; "
    << "qps: " << qps << std::endl;
  return 0;
}
