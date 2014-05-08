
#include <map>
#include <set>
#include <stdio.h>
#include <string>
#include <utility>

#include "thirdparty/boost/bind.hpp"

#include "thirdparty/glog/logging.h"

#include "muduo/base/Atomic.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/ThreadLocalSingleton.h"
#include "muduo/base/Timestamp.h"

#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/protobuf/ProtobufCodecLite.h"

#include "sandbox/pubsub/message.pb.h"

namespace {
using std::map;
using std::make_pair;
using std::set;
using std::string;
using muduo::AtomicInt64;
using muduo::MutexLock;
using muduo::MutexLockGuard;
using muduo::ThreadLocalSingleton;
using muduo::Timestamp;
using muduo::net::Buffer;
using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::MessagePtr;
using muduo::net::ProtobufCodecLite;
using muduo::net::TcpConnectionPtr;
using muduo::net::TcpServer;
}

namespace zerus {
namespace pubsub {

typedef set<string> ConnectionSubscription;

class Counters : boost::noncopyable {
public:
  void incrementInBoundMessage() {
    inBoundMessage.increment();
  }

  void incrementOutBoundMessage() {
    outBoundMessage.increment();
  }

  void addInBoundTraffic(int64_t bytes) {
    inBoundTraffic.add(bytes);
  }

  void addOutBoundTraffic(int64_t bytes) {
    outBoundTraffic.add(bytes);
  }

  int64_t getInBoundMessage() {
    return inBoundMessage.get();
  }

  int64_t getOutBoundMessage() {
    return outBoundMessage.get();
  }

  int64_t getInBoundTraffic() {
    return inBoundTraffic.get();
  }

  int64_t getOutBoundTraffic() {
    return outBoundTraffic.get();
  }

private:
  AtomicInt64 inBoundMessage;
  AtomicInt64 outBoundMessage;
  AtomicInt64 inBoundTraffic;
  AtomicInt64 outBoundTraffic;
};

class Topic : public muduo::copyable {
public:
  Topic(const string& topic) : topic_(topic) {}

  void add(const TcpConnectionPtr& conn) {
    audiences_.insert(conn);
  }

  void remove(const TcpConnectionPtr& conn) {
    audiences_.erase(conn);
  }

  void publish(
    const string& content,
    ProtobufCodecLite& codec,
    Counters& counters) {
    PubSubMessage message = makeMessage(content);
    for (auto it = audiences_.begin(); it != audiences_.end(); ++it) {
      muduo::net::Buffer buf;
      codec.fillEmptyBuffer(&buf, message);
      counters.addOutBoundTraffic(buf.readableBytes());
      counters.incrementOutBoundMessage();
      (*it)->send(&buf);
    }
  }

private:
  PubSubMessage makeMessage(const string& content) {
    PubSubMessage message;
    message.set_op(Op::PUB);
    message.set_topic(topic_);
    message.set_content(content);
    return message;
  }

  string topic_;
  set<TcpConnectionPtr> audiences_;
};

class PubSubServer : boost::noncopyable {
 public:
  PubSubServer(
    EventLoop* loop,
    const InetAddress& listenAddr) :
    server_(loop, listenAddr, "PubSubServer"),
    codec_(
      &PubSubMessage::default_instance(), 
      "ZR",
      boost::bind(&PubSubServer::onPubSubMessage, this, _1, _2, _3)
    ) {
    server_.setConnectionCallback(
      boost::bind(&PubSubServer::onConnection, this, _1)
    );
    server_.setMessageCallback(
      boost::bind(&PubSubServer::onMessage, this, _1, _2, _3)
    );
    loop->runEvery(1.0f, boost::bind(&PubSubServer::timePublish, this));
    loop->runEvery(10.0f, boost::bind(&PubSubServer::printCounters, this));
  }

  void start() {
    server_.setThreadInitCallback(
      boost::bind(&PubSubServer::threadInit, this, _1)
    );
    server_.start();
  }

  void setThreadNum(int numThreads) {
    server_.setThreadNum(numThreads);
  }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
      conn->setContext(ConnectionSubscription());
    } else {
      const ConnectionSubscription& connSub
        = boost::any_cast<const ConnectionSubscription&>(conn->getContext());
      // subtle: doUnsubscribe will erase *it, so increase before calling.
      for (auto it = connSub.begin(); it != connSub.end();) {
        doUnsubscribe(conn, *it++);
      }
    }
  }

  void onMessage(
    const TcpConnectionPtr& conn,
    Buffer* buf,
    Timestamp receiveTime) {
    counters_.addInBoundTraffic(buf->readableBytes());
    codec_.onMessage(conn, buf, receiveTime);
  }

  void onPubSubMessage(
    const TcpConnectionPtr& connectionPtr,
    const MessagePtr& messagePtr,
    Timestamp) {
    const PubSubMessage& message 
      = *dynamic_cast<PubSubMessage*>(messagePtr.get());
    counters_.incrementInBoundMessage();
    switch(message.op()) {
    case Op::PUB:
      distributePublish(message.topic(), message.content());
      break;
    case Op::SUB:
      doSubscribe(connectionPtr, message.topic());
      break;
    case Op::UNSUB:
      doUnsubscribe(connectionPtr, message.topic());
      break;
    default:
      LOG(ERROR) << "Unsupported message op: " << message.op();
      connectionPtr->shutdown();
      break;
    }
  }

  void timePublish() {
    Timestamp now = Timestamp::now();
    distributePublish("utc_time", string(now.toFormattedString().data()));
  }

  void printCounters() {
    int64_t inBoundTraffic = counters_.getInBoundTraffic();
    int64_t inBoundMessage = counters_.getInBoundMessage();
    int64_t outBoundTraffic = counters_.getOutBoundTraffic();
    int64_t outBoundMessage = counters_.getOutBoundMessage();

    int64_t deltaInBoundTraffic = inBoundTraffic - lastInBoundTraffic_;
    int64_t deltaInBoundMessage = inBoundMessage - lastInBoundMessage_;
    int64_t deltaOutBoundTraffic = outBoundTraffic - lastOutBoundTraffic_;
    int64_t deltaOutBoundMessage = outBoundMessage - lastOutBoundMessage_;

    lastInBoundTraffic_ = inBoundTraffic;
    lastInBoundMessage_ = inBoundMessage;
    lastOutBoundTraffic_ = outBoundTraffic;
    lastOutBoundMessage_ = outBoundMessage;

    LOG(INFO)
      << ((double)deltaInBoundTraffic) / 10.0f / 1024.0f / 1024.0f
      << " MB/s received, "
      << ((double)deltaInBoundMessage) / 10.0f
      << " messages/s received, "
      << ((double)deltaOutBoundTraffic) / 10.0f / 1024.0f / 1024.0f
      << " MB/s sent, "
      << ((double)deltaOutBoundMessage) / 10.0f
      << " messages/s sent.";
  }

  void doSubscribe(const TcpConnectionPtr& conn, const string& topic) {
    ConnectionSubscription* connSub
      = boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
    connSub->insert(topic);
    getTopic(topic).add(conn);
  }

  void doUnsubscribe(const TcpConnectionPtr& conn, const string& topic) {
    getTopic(topic).remove(conn);
    ConnectionSubscription* connSub
      = boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
    connSub->erase(topic);
  }

  void doPublish(const string& topic, const string& content) {
    getTopic(topic).publish(content, codec_, counters_);
  }

  void distributePublish(const string& topic, const string& content) {
    EventLoop::Functor f 
      = boost::bind(&PubSubServer::doPublish, this, topic, content);
    distribute(f);
  }

  void distribute(const EventLoop::Functor& f)
  {
    MutexLockGuard lock(mutex_);
    for (auto it = loops_.begin(); it != loops_.end(); ++it) {
      (*it)->queueInLoop(f);
    }
  }

  Topic& getTopic(const string& topic) {
    map<string, Topic>& localTopics = LocalTopics::instance();
    auto it = localTopics.find(topic);
    if (it == localTopics.end()) {
      it = localTopics.insert(make_pair(topic, Topic(topic))).first;
    }
    return it->second;
  }

  void threadInit(EventLoop* loop) {
    assert(LocalTopics::pointer() == NULL);
    LocalTopics::instance();
    assert(LocalTopics::pointer() != NULL);
    MutexLockGuard lock(mutex_);
    loops_.insert(loop);
  }

  TcpServer server_;
  ProtobufCodecLite codec_;
  typedef ThreadLocalSingleton<map<string, Topic>> LocalTopics;

  MutexLock mutex_;
  set<EventLoop*> loops_;

  Counters counters_;
  int64_t lastInBoundTraffic_ = 0;
  int64_t lastInBoundMessage_ = 0;
  int64_t lastOutBoundTraffic_ = 0;
  int64_t lastOutBoundMessage_ = 0;
};

} // namespace pubsub
} // namespace zerus

DEFINE_int32(
  port,
  8888,
  "Port of the hub"
);

int
main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  LOG(INFO) << "glog initialized";
  uint16_t port = static_cast<uint16_t>(FLAGS_port);
  EventLoop loop;
  zerus::pubsub::PubSubServer server(&loop, InetAddress(port));
  server.setThreadNum(8);
  server.start();
  loop.loop();
}
