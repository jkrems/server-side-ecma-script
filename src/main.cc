#include <iostream>

#include "uv.h"
#include "v8.h"

#include "node_v8_platform.h"

uv_timer_t my_timer;

static void HandleTimeout(uv_timer_t *handle) {
  std::cout << "Timer fired" << std::endl;
}

static void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() < 1) return;
  v8::HandleScope scope(args.GetIsolate());
  v8::Handle<v8::Value> arg = args[0];
  v8::String::Utf8Value value(arg);
  std::cout << *value << std::endl;
}

static void InitV8() {
  v8::V8::InitializeICU();
  v8::V8::InitializePlatform(new node::Platform(/* threadCount = */ 4));
  v8::V8::Initialize();
}

// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile(v8::Isolate* isolate, const std::string& name) {
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

int main(int argc, char *argv[]) {
  std::cout << "Hello World!" << std::endl;

  uv_loop_t *loop = uv_default_loop();

  uv_timer_init(loop, &my_timer);
  uv_timer_start(&my_timer, &HandleTimeout, 500, 0);

  InitV8();
  v8::Isolate* isolate = v8::Isolate::New();
  {
    v8::Isolate::Scope isolate_scope(isolate);

    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
    global->Set(v8::String::NewFromUtf8(isolate, "log"),
                v8::FunctionTemplate::New(isolate, LogCallback));

    v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);

    v8::Context::Scope context_scope(context);

    v8::Local<v8::String> source = ReadFile(isolate, "examples/hello.js");

    v8::Local<v8::Script> script = v8::Script::Compile(source);

    v8::Local<v8::Value> result = script->Run();

    v8::String::Utf8Value utf8(result);

    std::cout << *utf8 << std::endl;
  }

  uv_run(loop, UV_RUN_DEFAULT);

  std::cout << "Event loop finished" << std::endl;

  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();

  return 0;
}
