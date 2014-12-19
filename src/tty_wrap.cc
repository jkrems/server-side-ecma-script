#include <iostream>

#include "environment.h"
#include "tty_wrap.h"
#include "util.h"
#include "util-inl.h"

using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Handle;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::Persistent;
using v8::Promise;
using v8::String;
using v8::Value;

TTYWrap::TTYWrap(Handle<Object> object, Environment* env, uv_file fd, int readable) :
  env_(env),
  persistent_(env_->isolate(), object),
  handle_((uv_handle_t*)&tty_),
  stream_((uv_stream_t*)&tty_) {
  uv_tty_init(env_->loop(), &tty_, fd, readable);

  handle_->data = this;
  object->SetAlignedPointerInInternalField(0, this);
}

TTYWrap::~TTYWrap() {
}

void TTYWrap::Initialize(Handle<Object> target,
                         Environment* env) {
  Isolate *isolate = env->isolate();
  Local<FunctionTemplate> t = FunctionTemplate::New(isolate, New);
  Local<String> className = String::NewFromUtf8(isolate, "TTY");
  t->SetClassName(className);

  Local<ObjectTemplate> instance = t->InstanceTemplate();
  instance->SetInternalFieldCount(1);
  v8::Handle<v8::Signature> s = v8::Signature::New(isolate, t);
  v8::Local<FunctionTemplate> write =
    FunctionTemplate::New(isolate, Write, Handle<Value>(), s);
  instance->Set(String::NewFromUtf8(env->isolate(), "write"), write);

  target->Set(className, t->GetFunction());
}

void TTYWrap::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope scope(isolate);
  Environment *env = Environment::GetCurrent(isolate);

  uv_file fd = (uv_file)args[0]->Int32Value();

  /* TTYWrap *wrap = */
  new TTYWrap(args.This(), env, fd, args[1]->IsTrue());
}

class WriteResolver {
public:
  WriteResolver(Environment* env, int length) :
    length_(length),
    buf_(new uv_buf_t[length]),
    env_(env) {}

  void Set(int idx, char* base, int len) {
    buf_[idx].base = base;
    buf_[idx].len = len;
  }

  Handle<v8::Promise> Flush(uv_stream_t *stream) {
    Isolate *isolate = env_->isolate();
    HandleScope scope(isolate);

    Handle<Promise::Resolver> resolver = Promise::Resolver::New(isolate);
    resolver_.Reset(isolate, resolver);
    Handle<Promise> promise = resolver->GetPromise();
    promise_.Reset(isolate, promise);

    uv_write(&req_, stream, buf_, length_, &OnWritten);
    return node::PersistentToLocal(isolate, promise_);
  }

  static void OnWritten(uv_write_t *req, int status) {
    WriteResolver *self = (WriteResolver*) req;
    Isolate *isolate = self->isolate();
    HandleScope scope(isolate);

    Handle<Promise::Resolver> resolver = self->resolver();
    resolver->Resolve(v8::Undefined(isolate));

    delete self;
  }

  Handle<Promise::Resolver> resolver() {
    return node::PersistentToLocal(isolate(), resolver_);
  }

  Isolate *isolate() { return env_->isolate(); }

  ~WriteResolver() {
    if (buf_) delete[] buf_;
    buf_ = nullptr;
  }

protected:
  uv_write_t req_;
  int length_;
  uv_buf_t *buf_;

  Persistent<Promise::Resolver> resolver_;
  Persistent<Promise> promise_;

  Environment *env_;
};

void TTYWrap::Write(const FunctionCallbackInfo<Value>& args) {
  TTYWrap *wrap =
    static_cast<TTYWrap*>(args.Holder()->GetAlignedPointerFromInternalField(0));

  String::Utf8Value chunk(args[0]);

  WriteResolver *resolver = new WriteResolver(wrap->env_, 1);
  resolver->Set(0, *chunk, chunk.length());
  args.GetReturnValue().Set(resolver->Flush(wrap->stream_));
}
