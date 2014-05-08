
#include <atomic>
#include <iostream>
#include <stdio.h>

#include "thirdparty/boost/bind.hpp"

#include "thirdparty/glog/logging.h"

#include "muduo/base/ProcessInfo.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"

#include "PubSub.h"

namespace {
using namespace muduo;
using namespace muduo::net;
using namespace zerus::pubsub;
}

EventLoop* g_loop = NULL;
std::string g_topic;
std::string g_content;

void
onConnection(PubSubClient* client) {
  if (client->connected()) {
    LOG(INFO) << "connected";
    client->publish(g_topic, g_content);
    client->stop();
  } else {
    LOG(INFO) << "disconnected";
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
  topic,
  "t1",
  "Content to publish. Use - to read from unix standard input."
);

DEFINE_string(
  content,
  "-",
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
  g_topic = FLAGS_topic;
  g_content = FLAGS_content;

  std::string name(
    (ProcessInfo::username() +"@" + ProcessInfo::hostname()).data()
  );
  name += ":" + std::string(ProcessInfo::pidString().data());

  if (g_content == "-") {
    EventLoopThread loopThread;
    g_loop = loopThread.startLoop();
    PubSubClient client(g_loop, InetAddress(hostip, port), name);
    client.start();
    sleep(2);
    std::string line;
    while (getline(std::cin, line)) {
      client.publish(g_topic, line);
    }
    client.stop();
  } else {
    EventLoop loop;
    g_loop = &loop;
    PubSubClient client(g_loop, InetAddress(hostip, port), name);
    client.setConnectionCallback(onConnection);
    client.start();
    loop.loop();
  }
}
