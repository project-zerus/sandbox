proto_library(
  name = 'message_proto',
  srcs = 'message.proto'
)

cc_library(
  name = 'pubsub',
  extra_cppflags = '-std=c++0x',
  srcs = [
    'PubSub.cpp',
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
    'Pub.cpp',
  ],
  deps = ':pubsub',
)

cc_binary(
  name = 'sub',
  extra_cppflags = '-std=c++0x',
  srcs = [
    'Sub.cpp',
  ],
  deps = [
    ':pubsub',
    '//toft/base/string:string',
  ],
)

cc_binary(
  name = 'stresstest',
  extra_cppflags = '-std=c++0x',
  srcs = [
    'StressTest.cpp',
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
    'Hub.cpp',
  ],
  deps = [
    ':message_proto',
    '//essence/muduo:muduo',
  ],
)
