cc_binary(
    name = 'pns-server',
    srcs = [
        'pns-server.cc',
        'apns.cc',
    ],
    deps = [
        '//essence/libevhtp:libevhtp',
        '//thirdparty/libevent:libevent'
    ]
)

gen_rule(
    name = 'copy-certificates',
    cmd = 'cp $SRCS $OUTS',
    srcs = ['apns.pem'],
    outs = ['cert.pem']
)
