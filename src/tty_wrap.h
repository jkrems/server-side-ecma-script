#ifndef SRC_SSES_TTY_WRAP_H_
#define SRC_SSES_TTY_WRAP_H_

#include "v8.h"
#include "uv.h"

class Environment;

class TTYWrap {
public:
  static void Initialize(v8::Handle<v8::Object> target, Environment* env);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);

  TTYWrap(v8::Handle<v8::Object> object, Environment* env, uv_file fd, int readable);
  virtual ~TTYWrap();

protected:
  Environment *env_;
  v8::Persistent<v8::Object> persistent_;
  uv_handle_t *handle_;
  uv_stream_t *stream_;

  uv_tty_t tty_;
};

#endif // SRC_SSES_TTY_WRAP_H_
