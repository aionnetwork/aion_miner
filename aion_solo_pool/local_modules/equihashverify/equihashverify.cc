#include <nan.h>
#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <stdint.h>
#include <sodium.h>
#include "src/equi/equi210.h"

using namespace v8;

char hex_map[] = "0123456789abcdef";

extern "C" {
	#include "src/equi/blake2b.h"
}

void blake2b(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 2) {
		isolate->ThrowException(
			Exception::TypeError(
				String::NewFromUtf8(isolate, "Wrong number of arguments")
			)
		);
		return;
	}

	const uint8_t len = args[0]->Uint32Value();
	if(len != 32 && len != 64){
		isolate->ThrowException(
			Exception::TypeError(
				String::NewFromUtf8(isolate, "Invalid len argument. Only 32 or 64 allowed.")
			)
		);
		return;
	}

	Local<Object> input = args[1]->ToObject();
	if(!node::Buffer::HasInstance(input)) {
		isolate->ThrowException(
			Exception::TypeError(
				String::NewFromUtf8(isolate, "Arguments should be buffer objects.")
			)
		);
		return;
	} 

	const char *in = node::Buffer::Data(input);
	unsigned char out[len];
	bool success = blake2b(in, len, out);

	if(success){	
		char* hexOut = new char[len * 2 + 1];
		int offset = 0;
	    for(int i = 0; i < len; i++){
	 		hexOut[offset++] = hex_map[out[i] / 0x10];
	 		hexOut[offset++] = hex_map[out[i] % 0x10];
	    }
	    hexOut[offset++] = '\0';

		args.GetReturnValue().Set(String::NewFromUtf8(isolate, hexOut));
	}
}


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
  NODE_SET_METHOD(exports, "blake2b", blake2b);
}

NODE_MODULE(equihashverify, Init)
