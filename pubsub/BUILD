proto_library(
  name = 'message_proto',
  srcs = 'message.proto'
)

cc_library(
  name = 'pubsub',
  extra_cppflags = '-std=c++0x',
  srcs = [
    'pubsub.cc',
  ],
  deps = [
    ':message_proto',
    '//essence/muduo:muduo',
    '//thirdparty/glog:glog',
  ],
)

cc_binary(
  name = 'pub',
  extra_cppflags = '-std=c++0x',
  srcs = [
    'pub.cc',
  ],
  deps = ':pubsub',
)

cc_binary(
  name = 'sub',
  extra_cppflags = '-std=c++0x',
  srcs = [
    'sub.cc',
  ],
  deps = [
    ':pubsub',
    '//toft/base/string:string',
  ],
)

cc_binary(
  name = 'hub',
  extra_cppflags = '-std=c++0x',
  srcs = [
    'hub.cc',
  ],
  deps = [
    ':message_proto',
    '//essence/muduo:muduo',
  ],
)
