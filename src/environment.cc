#include "environment.h"

#include <iostream>

static void Log(const v8::FunctionCallbackInfo<v8::Value>& args) {
  using v8::HandleScope;
  using v8::Handle;
  using v8::Value;
  using v8::String;

  if (args.Length() < 1) return;

  HandleScope scope(args.GetIsolate());
  Handle<Value> arg = args[0];
  String::Utf8Value value(arg);
  std::cout << *value << std::endl;
}

Environment::Environment() {
}

bool Environment::Init() {
  using v8::Isolate;
  using v8::HandleScope;
  using v8::Local;
  using v8::ObjectTemplate;
  using v8::Context;

  uv_loop_init(&loop_);

  isolate_ = Isolate::New();
  isolate_->SetAutorunMicrotasks(true);
  {
    Isolate::Scope isolate_scope(isolate_);

    HandleScope handle_scope(isolate_);

    Local<v8::ObjectTemplate> global = ObjectTemplate::New();
    global->Set(v8::String::NewFromUtf8(isolate_, "log"),
                v8::FunctionTemplate::New(isolate_, Log));

    v8::Local<v8::Context> context = v8::Context::New(isolate_, NULL, global);
    context_.Reset(isolate_, context);
    AssignToContext(context);

    v8::Context::Scope context_scope(context);

    v8::Local<v8::FunctionTemplate> process_template =
      v8::FunctionTemplate::New(isolate_);
    process_template->SetClassName(
      v8::String::NewFromUtf8(isolate_, "process"));

    v8::Local<v8::Object> process_object =
      process_template->GetFunction()->NewInstance();

    process_.Reset(isolate_, process_object);
  }
  return true;
}

void Environment::SetMethod(v8::Local<v8::Object> that,
                            const char* name,
                            v8::FunctionCallback callback) {
  v8::Local<v8::Function> function =
      v8::FunctionTemplate::New(isolate(), callback)->GetFunction();
  v8::Local<v8::String> name_string = v8::String::NewFromUtf8(isolate(), name);
  that->Set(name_string, function);
  function->SetName(name_string);  // NODE_SET_METHOD() compatibility.
}

void Environment::AssignToContext(v8::Local<v8::Context> context) {
  context->SetAlignedPointerInEmbedderData(kContextEmbedderDataIndex, this);
}

Environment* Environment::GetCurrent(v8::Isolate* isolate) {
  return GetCurrent(isolate->GetCurrentContext());
}

Environment* Environment::GetCurrent(v8::Local<v8::Context> context) {
  return static_cast<Environment*>(
      context->GetAlignedPointerFromEmbedderData(kContextEmbedderDataIndex));
}

v8::Local<v8::Context> Environment::context() {
  return v8::Local<v8::Context>::New(isolate(), context_);
}

v8::Local<v8::Object> Environment::process() {
  return v8::Local<v8::Object>::New(isolate(), process_);
}
