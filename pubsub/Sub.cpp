
#include <stdio.h>
#include <string>
#include <vector>

#include "toft/base/string/algorithm.h"

#include "thirdparty/boost/bind.hpp"

#include "thirdparty/glog/logging.h"

#include "essence/muduo/base/ProcessInfo.h"
#include "essence/muduo/net/EventLoop.h"

#include "PubSub.h"

namespace {
using namespace muduo;
using namespace muduo::net;
using namespace zerus::pubsub;
}

EventLoop* g_loop = NULL;
std::vector<std::string> g_topics;

void
onSubscription(
  const std::string& topic,
  const std::string& content,
  Timestamp) {
  printf("%s: %s\n", topic.c_str(), content.c_str());
}

void
onConnection(PubSubClient* client) {
  if (client->connected()) {
    for (auto it = g_topics.begin(); it != g_topics.end(); ++it) {
      client->subscribe(*it, onSubscription);
    }
  } else {
    g_loop->quit();
  }
}

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

DEFINE_string(
  topics,
  "t1,t2",
  "Content to publish. Use - to read from unix standard input."
);

int
main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  LOG(INFO) << "glog initialized";

  std::string hostip = FLAGS_ip;
  uint16_t port = static_cast<uint16_t>(FLAGS_port);

  g_topics.clear();
  toft::SplitString(FLAGS_topics, ",", &g_topics);
  LOG(INFO) << "Topics: " << FLAGS_topics;

  EventLoop loop;
  g_loop = &loop;
  std::string name(
    (ProcessInfo::username() + "@" + ProcessInfo::hostname()).data()
  );
  name += ":" + std::string(ProcessInfo::pidString().data());
  PubSubClient client(&loop, InetAddress(hostip, port), name);
  client.setConnectionCallback(onConnection);
  client.start();
  loop.loop();
}
