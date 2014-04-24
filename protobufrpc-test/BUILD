proto_library(
    name = 'echo_proto',
    srcs = 'echo.proto'
)

cc_binary(
    name = 'server',
    srcs = [
        'server.cc',
    ],
    deps = [
        ':echo_proto',
        '//essence/muduo:muduo',
    ]
)

cc_binary(
    name = 'client',
    srcs = [
        'client.cc',
    ],
    deps = [
        ':echo_proto',
        '//essence/muduo:muduo',
        '//toft/system/time:time',
    ]
)
