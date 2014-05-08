
#include <string>

#include "thirdparty/boost/bind.hpp"

#include "thirdparty/glog/logging.h"

#include "sandbox/pubsub/message.pb.h"

#include "PubSub.h"

namespace {
using std::string;
using muduo::Timestamp;
using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::MessagePtr;
using muduo::net::ProtobufCodecLite;
using muduo::net::TcpConnectionPtr;
using muduo::net::TcpClient;
}

namespace zerus {
namespace pubsub {

PubSubClient::PubSubClient(
  EventLoop* loop,
  const InetAddress& hubAddr,
  const std::string& name) : 
  client_(loop, hubAddr, muduo::string(name.data())),
  codec_(
    &PubSubMessage::default_instance(), 
    "ZR",
    boost::bind(&PubSubClient::onPubSubMessage, this, _1, _2, _3)
  ) {

  client_.setConnectionCallback(
    boost::bind(&PubSubClient::onConnection, this, _1)
  );

  client_.setMessageCallback(
    boost::bind(&ProtobufCodecLite::onMessage, &codec_, _1, _2, _3)
  );
}

void
PubSubClient::start() {
  client_.connect();
}

void
PubSubClient::stop() {
  client_.disconnect();
}

bool
PubSubClient::connected() const {
  return conn_ && conn_->connected();
}

bool
PubSubClient::subscribe(const std::string& topic, const SubscribeCallback& cb) {
  DLOG(INFO) << "Subscribe to topic: " << topic;
  subscribeCallback_ = cb;
  PubSubMessage message;
  message.set_op(Op::SUB);
  message.set_topic(topic);
  codec_.send(conn_, message);
  return true;
}

void
PubSubClient::unsubscribe(const string& topic) {
  PubSubMessage message;
  message.set_op(Op::UNSUB);
  message.set_topic(topic);
  codec_.send(conn_, message);
}

bool
PubSubClient::publish(const string& topic, const string& content) {
  PubSubMessage message;
  message.set_op(Op::PUB);
  message.set_topic(topic);
  message.set_content(content);
  codec_.send(conn_, message);
  return true;
}

void
PubSubClient::onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn_ = conn;
  } else {
    conn_.reset();
  }
  if (connectionCallback_) {
    connectionCallback_(this);
  }
}

void
PubSubClient::onPubSubMessage(
  const muduo::net::TcpConnectionPtr& connectionPtr,
  const muduo::net::MessagePtr& messagePtr,
  muduo::Timestamp receiveTime) {
  const PubSubMessage& message 
    = *dynamic_cast<PubSubMessage*>(messagePtr.get());
  switch(message.op()) {
  case Op::PUB:
    if (subscribeCallback_) {
      subscribeCallback_(message.topic(), message.content(), receiveTime);
    }
    break;
  default:
    LOG(ERROR) << "Unsupported message op: " << message.op();
    break;
  }
}

} // namespace pubsub
} // namespace zerus
