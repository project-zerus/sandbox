
#include <stdio.h>
#include <string>
#include <thread>
#include <vector>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

#include "toft/base/string/algorithm.h"

#include "thirdparty/boost/bind.hpp"

#include "thirdparty/glog/logging.h"

#include "essence/muduo/base/ProcessInfo.h"
#include "essence/muduo/net/EventLoop.h"

#include "PubSub.h"

DEFINE_string(
  ip,
  "127.0.0.1",
  "IP of the hub"
);

DEFINE_int32(
  port,
  8888,
  "Port of the hub"
);

DEFINE_int32(
  threads,
  8,
  "Number of threads."
);

DEFINE_int32(
  connections,
  10,
  "Connections per thread."
);

DEFINE_string(
  topics,
  "t1,t2,t3,t4,t5",
  "Content to publish. Use - to read from unix standard input."
);

DEFINE_int32(
  message_size,
  1024,
  "Message size."
);

namespace {

using namespace muduo;
using namespace muduo::net;
using namespace zerus::pubsub;

class StressTest : boost::noncopyable {
public:
  StressTest(
    std::string hostip,
    uint16_t port,
    std::string name,
    std::vector<std::string> topics,
    uint16_t numberOfConnections) :
    hostip_(hostip),
    port_(port),
    name_(name),
    topics_(topics),
    numberOfConnections_(numberOfConnections) {
  }

  void run() {
    EventLoop loop;
    std::vector<std::shared_ptr<PubSubClient>> clients;
    for (int i = 0; i < numberOfConnections_; ++i) {
      std::shared_ptr<PubSubClient> client(
        new PubSubClient(&loop, InetAddress(hostip_, port_), name_)
      );
      client->setConnectionCallback(
        boost::bind(&StressTest::onConnection, this, _1)
      );
      client->setWriteCompleteCallback(
        boost::bind(&StressTest::onWriteComplete, this, _1)
      );
      client->start();
      clients.push_back(client);
    }
    loop.loop();
  }

private:
  void onSubscription(
    const std::string& topic,
    const std::string& content,
    Timestamp) {
  }

  void onConnection(PubSubClient* client) {
    if (client->connected()) {
      for (auto it = topics_.begin(); it != topics_.end(); ++it) {
        client->subscribe(
          *it, boost::bind(&StressTest::onSubscription, this, _1, _2, _3)
        );
      }
    } else {
      client->stop();
    }
  }

  void onWriteComplete(PubSubClient* client) {
    std::string message;
    for (auto i = 0; i < FLAGS_message_size; ++i) {
      boost::random::uniform_int_distribution<> dist(0, 9);
      message += std::to_string(dist(gen_));
    }
    for (auto& topic : topics_) {
      client->publish(topic, message);
    }
  }

  std::string hostip_;
  uint16_t port_;
  std::string name_;
  std::vector<std::string> topics_;
  boost::random::mt19937 gen_;
  const uint16_t numberOfConnections_;
};


}

int
main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  LOG(INFO) << "glog initialized";

  std::string hostip = FLAGS_ip;
  uint16_t port = static_cast<uint16_t>(FLAGS_port);

  std::vector<std::string> topics;
  toft::SplitString(FLAGS_topics, ",", &topics);
  LOG(INFO) << "Topics: " << FLAGS_topics;

  std::string name(
    (ProcessInfo::username() + "@" + ProcessInfo::hostname()).data()
  );
  name += ":" + std::string(ProcessInfo::pidString().data());

  std::vector<std::shared_ptr<std::thread>> threads;
  std::vector<std::shared_ptr<StressTest>> tests;
  for (int i = 0; i < FLAGS_threads; ++i) {
    std::shared_ptr<StressTest> test(
      new StressTest(hostip, port, name, topics, FLAGS_connections)
    );
    std::shared_ptr<std::thread> thread(
      new std::thread(std::bind(&StressTest::run, test.get()))
    );
    tests.push_back(test);
    threads.push_back(thread);
  }
  for (auto thread : threads) {
    thread->join();
  }
  return 0;
}
