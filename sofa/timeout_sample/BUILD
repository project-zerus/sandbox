proto_library(
  name = 'sleep_service_proto',
  srcs = 'sleep_service.proto'
)

cc_binary(
  name = 'server',
  srcs = [
    'server.cc',
  ],
  deps = [
    ':sleep_service_proto',
    '//sofa-pbrpc:sofa-pbrpc',
  ]
)

cc_binary(
  name = 'client',
  srcs = [
    'client.cc',
  ],
  deps = [
    ':sleep_service_proto',
    '//sofa-pbrpc:sofa-pbrpc',
  ]
)
