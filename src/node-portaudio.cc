#include "napi.h"

#include "./getDevices.h"
#include "./output.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {

  exports.Set(Napi::String::New(env, "getDevices"), Napi::Function::New(env, bma::GetDevices));
  exports.Set(Napi::String::New(env, "createOutput"), Napi::Function::New(env, bma::CreateOutput));

  return exports;

}

NODE_API_MODULE(addon, Init)
