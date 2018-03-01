#include <nan.h>
#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <stdint.h>
#include "src/equi/equi210.h"

using namespace v8;


void Verify(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.Length() < 2) {
  isolate->ThrowException(Exception::TypeError(
    String::NewFromUtf8(isolate, "Wrong number of arguments")));
  return;
  }

  Local<Object> header = args[0]->ToObject();
  Local<Object> solution = args[1]->ToObject();

  if(!node::Buffer::HasInstance(header) || !node::Buffer::HasInstance(solution)) {
  isolate->ThrowException(Exception::TypeError(
    String::NewFromUtf8(isolate, "Arguments should be buffer objects.")));
  return;
  }

  const char *hdr = node::Buffer::Data(header);
  const char *soln = node::Buffer::Data(solution);

  int n = 210;
  int k = 9;

  bool result = verifyEH(hdr, soln, n, k);
  args.GetReturnValue().Set(result);

}

void Init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "verify", Verify);
}

NODE_MODULE(equihashverify, Init)
