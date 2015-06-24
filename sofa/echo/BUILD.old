proto_library(
  name = 'echo_service_proto',
  srcs = 'echo_service.proto'
)

cc_binary(
  name = 'echo_client_async',
  srcs = [
    'client_async.cc',
  ],
  deps = [
    ':echo_service_proto',
    '//sofa-pbrpc:sofa-pbrpc'
  ]
)

cc_binary(
  name = 'echo_client_sync',
  srcs = [
    'client_sync.cc',
  ],
  deps = [
    ':echo_service_proto',
    '//sofa-pbrpc:sofa-pbrpc'
  ]
)

cc_binary(
  name = 'echo_server',
  srcs = [
    'server.cc',
  ],
  deps = [
    ':echo_service_proto',
    '//sofa-pbrpc:sofa-pbrpc'
  ]
)

cc_binary(
  name = 'log_test',
  srcs = [
    'log_test.cc',
  ],
  deps = [
    '//sofa-pbrpc:sofa-pbrpc',
    '//thirdparty/glog:glog',
  ]
)
