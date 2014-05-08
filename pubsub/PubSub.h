#ifndef ZERUS_PUBSUB_H
#define ZERUS_PUBSUB_H

#include "muduo/net/TcpClient.h"
#include "muduo/net/protobuf/ProtobufCodecLite.h"

namespace zerus {
namespace pubsub {

class PubSubClient : boost::noncopyable {
 public:
  typedef boost::function<void (PubSubClient*)> ConnectionCallback;
  
  typedef boost::function<
    void (
      const std::string& topic,
      const std::string& content,
      muduo::Timestamp
    )
  > SubscribeCallback;

  PubSubClient(
    muduo::net::EventLoop* loop,
    const muduo::net::InetAddress& hubAddr,
    const std::string& name
  );

  void start();
  void stop();
  bool connected() const;

  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }

  bool subscribe(const std::string& topic, const SubscribeCallback& cb);
  void unsubscribe(const std::string& topic);
  bool publish(const std::string& topic, const std::string& content);

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn);

  void onPubSubMessage(
    const muduo::net::TcpConnectionPtr& connectionPtr,
    const muduo::net::MessagePtr& messagePtr,
    muduo::Timestamp
  );

  muduo::net::TcpClient client_;
  muduo::net::TcpConnectionPtr conn_;

  muduo::net::ProtobufCodecLite codec_;

  ConnectionCallback connectionCallback_;
  SubscribeCallback subscribeCallback_;
};

} // pubsub
} // zerus

#endif  // ZERUS_PUBSUB_H
