cc_binary(
  name = 'echo-client-cpp',
  extra_cppflags = [
    '-std=c++11',
  ],
  srcs = [
    'main.cpp',
  ],
  deps = [
    '//sandbox/echo/echo-thrift-api:echo-thrift-api',
    '//toft/system/time:time',
  ]
)
