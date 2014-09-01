
namespace cpp zerus.sandbox.thrift_vs_protobuf.thrift

struct MetaData {
  1: i64 id,
  2: i32 size,
  3: string path
}

struct Data {
  1: i64 size,
  2: i32 offset,
  3: binary payload,
  4: bool eof
}
