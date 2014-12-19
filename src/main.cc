#include <iostream>

#include "uv.h"
#include "v8.h"

#include "node_v8_platform.h"
#include "environment.h"
#include "tty_wrap.h"

// static void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
//   if (args.Length() < 1) return;
//   v8::HandleScope scope(args.GetIsolate());
//   v8::Handle<v8::Value> arg = args[0];
//   v8::String::Utf8Value value(arg);
//   std::cout << *value << std::endl;
// }

static void InitV8() {
  const char enable_es6[] =
    "--harmony --harmony_arrow_functions";
  v8::V8::SetFlagsFromString(enable_es6, sizeof(enable_es6) - 1);
  v8::V8::InitializeICU();
  v8::V8::InitializePlatform(new node::Platform(/* threadCount = */ 4));
  v8::V8::Initialize();
}

// Reads a file into a v8 string.
static v8::Handle<v8::String> ReadFile(v8::Isolate* isolate, const std::string& name) {
  FILE* file = fopen(name.c_str(), "rb");
  if (file == NULL) return v8::Handle<v8::String>();

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = static_cast<int>(fread(&chars[i], 1, size - i, file));
    i += read;
  }
  fclose(file);
  v8::Handle<v8::String> result =
          v8::String::NewFromUtf8(isolate, chars, v8::String::kNormalString, size);
  delete[] chars;
  return result;
}

static v8::Local<v8::Value> RunFile(Environment *env, const std::string& name) {
  v8::Local<v8::String> source = ReadFile(env->isolate(), name);

  v8::Local<v8::Script> script = v8::Script::Compile(source);

  return script->Run();
}

static void SetMethod(v8::Isolate* isolate,
                      v8::Local<v8::Object> that,
                      const char* name,
                      v8::FunctionCallback callback) {
  v8::Local<v8::Function> function =
      v8::FunctionTemplate::New(isolate, callback)->GetFunction();
  v8::Local<v8::String> name_string = v8::String::NewFromUtf8(isolate, name);
  that->Set(name_string, function);
  function->SetName(name_string);  // NODE_SET_METHOD() compatibility.
}

template <class TypeName>
inline v8::Local<TypeName> StrongPersistentToLocal(
    const v8::Persistent<TypeName>& persistent) {
  return *reinterpret_cast<v8::Local<TypeName>*>(
      const_cast<v8::Persistent<TypeName>*>(&persistent));
}

template <class TypeName>
inline v8::Local<TypeName> WeakPersistentToLocal(
    v8::Isolate* isolate,
    const v8::Persistent<TypeName>& persistent) {
  return v8::Local<TypeName>::New(isolate, persistent);
}

template <class TypeName>
inline v8::Local<TypeName> PersistentToLocal(
    v8::Isolate* isolate,
    const v8::Persistent<TypeName>& persistent) {
  if (persistent.IsWeak()) {
    return WeakPersistentToLocal(isolate, persistent);
  } else {
    return StrongPersistentToLocal(persistent);
  }
}

class DelayedResolver {
public:
  DelayedResolver(Environment *env,
                  v8::Handle<v8::Promise::Resolver> resolver,
                  v8::Handle<v8::Promise> promise,
                  int msec) :
        handle__((uv_handle_t*)&timer_), msec_(msec),
        env_(env) {
    uv_timer_init(env->loop(), &timer_);
    uv_timer_start(&timer_, &OnTimeout, msec_, 0);
    handle__->data = this;
    resolver_.Reset(isolate(), resolver);
    promise_.Reset(isolate(), promise);
  }

protected:
  static void OnClose(uv_handle_t* handle) {
    DelayedResolver* wrap = static_cast<DelayedResolver*>(handle->data);
    // Environment* env = wrap->env();
    // v8::HandleScope scope(env->isolate());
    delete wrap;
  }

  static void OnTimeout(uv_timer_t *handle) {
    DelayedResolver* wrap = static_cast<DelayedResolver*>(handle->data);
    Environment* env = wrap->env();
    v8::HandleScope scope(env->isolate());
    v8::Local<v8::Promise::Resolver> resolver = wrap->resolver();
    resolver->Resolve(v8::Undefined(env->isolate()));
  }

  v8::Local<v8::Promise::Resolver> resolver() {
    return PersistentToLocal(isolate(), resolver_);
  }

  v8::Isolate *isolate() { return env_->isolate(); }
  Environment *env() { return env_; }

  uv_handle_t* handle__;

private:
  int msec_;
  uv_timer_t timer_;
  v8::Persistent<v8::Promise::Resolver> resolver_;
  v8::Persistent<v8::Promise> promise_;

  Environment *env_;
};

static void RunFileJS(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() < 1) return;
  v8::Isolate *isolate = args.GetIsolate();
  Environment *env = Environment::GetCurrent(isolate);
  v8::HandleScope scope(isolate);

  v8::Handle<v8::Value> arg = args[0];
  v8::String::Utf8Value filename(arg);

  v8::Handle<v8::Value> result = RunFile(env, *filename);

  args.GetReturnValue().Set(result);
}

static void Delay(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() < 1) return;
  v8::Isolate *isolate = args.GetIsolate();
  Environment *env = Environment::GetCurrent(isolate);
  v8::HandleScope scope(isolate);
  v8::Handle<v8::Value> arg = args[0];

  v8::Handle<v8::Promise::Resolver> resolver =
    v8::Promise::Resolver::New(isolate);

  v8::Handle<v8::Promise> promise = resolver->GetPromise();

  int msec = arg->Int32Value();
  new DelayedResolver(env, resolver, promise, msec);

  args.GetReturnValue().Set(promise);
}

static void Binding(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() < 1) return;
  v8::Isolate* isolate = args.GetIsolate();
  Environment *env = Environment::GetCurrent(isolate);
  v8::HandleScope scope(isolate);

  v8::Handle<v8::Value> arg = args[0];
  v8::String::Utf8Value value(arg);

  v8::Local<v8::Object> exports(v8::Object::New(isolate));
  if (!strcmp(*value, "timers")) {
    SetMethod(isolate, exports, "delay", Delay);
  } else if (!strcmp(*value, "script")) {
    SetMethod(isolate, exports, "runFile", RunFileJS);
  } else if (!strcmp(*value, "tty")) {
    TTYWrap::Initialize(exports, env);
  }

  args.GetReturnValue().Set(exports);
}

static v8::Local<v8::Value> RunInit(Environment *env) {
  v8::Handle<v8::Context> context = env->context();

  v8::Handle<v8::Value> init_value = RunFile(env, "lib/_init.js");
  v8::Handle<v8::Function> init_fn =
    v8::Handle<v8::Function>::Cast(init_value);

  // READONLY_PROPERTY(process_template, "pid", v8::Integer::New(isolate, getpid()));

  v8::Local<v8::Object> process = env->process();
  env->SetMethod(process, "binding", Binding);

  const int argc = 1;
  v8::Handle<v8::Value> argv[argc] = { process };
  v8::Handle<v8::Value> result =
    init_fn->Call(context->Global(), argc, argv);

  return result;
}

int main(int argc, char *argv[]) {
  InitV8();

  Environment env;
  if (!env.Init()) {
    return 1;
  }

  uv_loop_t *loop = env.loop();
  v8::Isolate* isolate = env.isolate();

  {
    v8::Isolate::Scope isolate_scope(isolate);

    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Context> context = env.context();

    v8::Context::Scope context_scope(context);

    RunInit(&env);

    RunFile(&env, "examples/hello.js");


    bool more;
    do {
      more = uv_run(loop, UV_RUN_ONCE);
      isolate->RunMicrotasks();
      if (more == false) {
        // Emit `beforeExit` if the loop became alive either after emitting
        // event, or after running some callbacks.
        more = uv_loop_alive(loop);
        if (uv_run(loop, UV_RUN_NOWAIT) != 0)
          more = true;
      }
    } while (more == true);
  }

  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();

  return 0;
}
