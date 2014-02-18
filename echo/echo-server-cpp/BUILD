cc_binary(
    name = 'echo-server-cpp',
    extra_cppflags = [
        '-std=c++0x',
    ],
    srcs = [
        'main.cpp',
        'EchoHandler.cpp',
    ],
    deps = ['//sandbox/echo/echo-thrift-api:echo-thrift-api']
)
