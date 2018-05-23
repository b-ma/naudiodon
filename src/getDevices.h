#pragma once

#include "napi.h"
#include "portaudio.h"

namespace bma {

Napi::Array GetDevices(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  uint32_t numDevices;

  PaError errCode = Pa_Initialize();

  if (errCode != paNoError) {
    std::string err = std::string("Could not initialize PortAudio: ") + Pa_GetErrorText(errCode);
    Napi::TypeError::New(env, err.c_str()).ThrowAsJavaScriptException();
  }

  numDevices = Pa_GetDeviceCount();
  Napi::Array result = Napi::Array::New(env, numDevices);

  for (uint32_t i = 0; i < numDevices; ++i) {
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);

    Napi::Object info = Napi::Object::New(env);

    info.Set("id", Napi::Number::New(env, i));
    info.Set("name", Napi::String::New(env, deviceInfo->name));
    info.Set("maxInputChannels", Napi::Number::New(env, deviceInfo->maxInputChannels));
    info.Set("maxOutputChannels", Napi::Number::New(env, deviceInfo->maxOutputChannels));
    info.Set("defaultSampleRate", Napi::Number::New(env, deviceInfo->defaultSampleRate));
    info.Set("defaultLowInputLatency", Napi::Number::New(env, deviceInfo->defaultLowInputLatency));
    info.Set("defaultLowOutputLatency", Napi::Number::New(env, deviceInfo->defaultLowOutputLatency));
    info.Set("defaultHighInputLatency", Napi::Number::New(env, deviceInfo->defaultHighInputLatency));
    info.Set("defaultHighOutputLatency", Napi::Number::New(env, deviceInfo->defaultHighOutputLatency));
    info.Set("hostAPIName", Napi::String::New(env, Pa_GetHostApiInfo(deviceInfo->hostApi)->name));

    result.Set(i, info);
  }

  Pa_Terminate();

  return result;
}

} // namespace

