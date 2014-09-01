
#include "glog/logging.h"

#include "toft/base/benchmark.h"

#include "sandbox/thrift_vs_protobuf/protobuf/Messages.pb.h"

#include "sandbox/thrift_vs_protobuf/thrift/Messages_constants.h"
#include "sandbox/thrift_vs_protobuf/thrift/Messages_types.h"

#include <boost/shared_ptr.hpp>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/protocol/TBinaryProtocol.h>

using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

template<typename T>
void thriftParseFromString(T& tag, const std::string& s) {
  boost::shared_ptr<TMemoryBuffer> trans(
    new TMemoryBuffer((uint8_t*) s.data(), s.size())
  );
  boost::shared_ptr<TProtocol> proto(new TBinaryProtocol(trans));
  tag.read(proto.get());
}

template<typename T>
void thriftSerializeToString(const T& tag, std::string& s) {
  boost::shared_ptr<TMemoryBuffer> trans(new TMemoryBuffer());
  boost::shared_ptr<TProtocol> proto(new TBinaryProtocol(trans));
  tag.write(proto.get());
  s = trans->getBufferAsString();
}

void MetaDataProtobufBench(int n) {
  using zerus::sandbox::thrift_vs_protobuf::protobuf::MetaData;
  MetaData metaData;
  metaData.set_id(0x12345678);
  metaData.set_size(0xabcd);
  metaData.set_path("/this/is/a/path/for/testing/aha/foo/bar/你好/bonjour");
  std::string bytes;
  for (int i = 0; i < n; ++i) {
    metaData.SerializeToString(&bytes);
    metaData.ParseFromString(bytes);
  }
  LOG(INFO) << "MetaData Protobuf size: " << bytes.size();
}

void MetaDataThriftBench(int n) {
  using zerus::sandbox::thrift_vs_protobuf::thrift::MetaData;
  MetaData metaData;
  metaData.id = 0x12345678;
  metaData.size = 0xabcd;
  metaData.path = "/this/is/a/path/for/testing/aha/foo/bar/你好/bonjour";
  std::string bytes;
  for (int i = 0; i < n; ++i) {
    thriftSerializeToString(metaData, bytes);
    thriftParseFromString(metaData, bytes);
  }
  LOG(INFO) << "MetaData Thrift size: " << bytes.size();
}

void DataProtobufBench(int n) {
  using zerus::sandbox::thrift_vs_protobuf::protobuf::Data;
  Data data;
  data.set_size(0x12345678);
  data.set_offset(0xabcd);
  std::string payload;
  payload.resize(1024, 'a');
  data.set_payload(payload);
  data.set_eof(true);
  std::string bytes;
  for (int i = 0; i < n; ++i) {
    data.SerializeToString(&bytes);
    data.ParseFromString(bytes);
  }
  LOG(INFO) << "Data Protobuf size: " << bytes.size();
}

void DataThriftBench(int n) {
  using zerus::sandbox::thrift_vs_protobuf::thrift::Data;
  Data data;
  data.size = 0x12345678;
  data.offset = 0xabcd;
  std::string payload;
  payload.resize(1024, 'a');
  data.payload = payload;
  data.eof = true;
  std::string bytes;
  for (int i = 0; i < n; ++i) {
    thriftSerializeToString(data, bytes);
    thriftParseFromString(data, bytes);
  }
  LOG(INFO) << "Data Thrift size: " << bytes.size();
}

TOFT_BENCHMARK(MetaDataProtobufBench)->ThreadRange(1, NumCPUs());

TOFT_BENCHMARK(MetaDataThriftBench)->ThreadRange(1, NumCPUs());

TOFT_BENCHMARK(DataProtobufBench)->ThreadRange(1, NumCPUs());

TOFT_BENCHMARK(DataThriftBench)->ThreadRange(1, NumCPUs());
