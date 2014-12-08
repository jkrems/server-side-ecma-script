#ifndef SRC_SSES_ENVIRONMENT_H_
#define SRC_SSES_ENVIRONMENT_H_

#include "v8.h"
#include "uv.h"

// Pick an index that's hopefully out of the way when we're embedded inside
// another application. Performance-wise or memory-wise it doesn't matter:
// Context::SetAlignedPointerInEmbedderData() is backed by a FixedArray,
// worst case we pay a one-time penalty for resizing the array.
#ifndef SSES_CONTEXT_EMBEDDER_DATA_INDEX
#define SSES_CONTEXT_EMBEDDER_DATA_INDEX 33
#endif

class Environment {
public:
  Environment();

  bool Init();

  inline v8::Isolate* isolate() { return isolate_; }
  inline uv_loop_t* loop() { return &loop_; }
  v8::Local<v8::Context> context();
  v8::Local<v8::Object> process();

  void SetMethod(v8::Local<v8::Object> that,
                 const char* name,
                 v8::FunctionCallback callback);


  static Environment* GetCurrent(v8::Isolate* isolate);
  static Environment* GetCurrent(v8::Local<v8::Context> context);

  enum ContextEmbedderDataIndex {
    kContextEmbedderDataIndex = SSES_CONTEXT_EMBEDDER_DATA_INDEX
  };

private:
  void AssignToContext(v8::Local<v8::Context> context);

  v8::Isolate* isolate_;
  v8::Persistent<v8::Context> context_;
  v8::Persistent<v8::Object> process_;

  uv_loop_t loop_;
};

#endif // SRC_SSES_ENVIRONMENT_H_
