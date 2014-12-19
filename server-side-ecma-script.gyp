{
  'targets': [{
    'target_name': 'server-side-ecma-script',
    'type': 'executable',
    'dependencies': [
      'deps/uv/uv.gyp:libuv',
      'deps/v8/tools/gyp/v8.gyp:v8',
    ],
    'cflags_cc': [ '-fno-rtti', '-fno-exceptions', '-std=gnu++0x' ],
    'sources': [
      'src/environment.cc',
      'src/main.cc',
      'src/node_v8_platform.cc',
      'src/tty_wrap.cc',
      'src/util.cc',
    ],
  }],
}
