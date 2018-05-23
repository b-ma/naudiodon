#pragma once

// notes
// - https://github.com/nodejs/node/issues/5692 (simple example)

#include <iostream>
#include <memory>
#include <vector>

#include "napi.h"
#include "uv.h"
#include "portaudio.h"

#include "./AudioOptions.h"

namespace bma {

PaStream* paOutputStream;
uv_async_t asyncHandle;
Napi::FunctionReference persistentOutputCallback;

struct data_t {
  std::shared_ptr<Napi::Env> env;
  std::shared_ptr<AudioOptions> options;
  std::shared_ptr<std::vector<float>> buffers[2];
};

data_t asyncOuputData;

// ----------------------------------------------------------------------
// PA WRAPPER
// ----------------------------------------------------------------------

uint32_t outputBufferCounter = 0; // flag for double buffering

void initPaOutputStream(Napi::Env env, AudioOptions audioOptions, PaStreamCallback *cb) {
  PaError errCode = Pa_Initialize();
  std::string err;

  if (errCode != paNoError) {
    err = std::string("Could not initialize PortAudio: ") + Pa_GetErrorText(errCode);
    Napi::TypeError::New(env, err.c_str()).ThrowAsJavaScriptException();
  }

  std::cout << audioOptions.toString("output").c_str() << std::endl;

  PaStreamParameters outParams;
  // memset(&outParams, 0, sizeof(PaStreamParameters));

  int deviceId = audioOptions.deviceId;

  if (deviceId >= 0 && deviceId < Pa_GetDeviceCount())
    outParams.device = (PaDeviceIndex) deviceId;
  else
    outParams.device = Pa_GetDefaultOutputDevice();

  if (outParams.device == paNoDevice) {
    err = std::string("No default output device");
    Napi::TypeError::New(env, err.c_str()).ThrowAsJavaScriptException();
  }

  printf("Output device name is %s\n", Pa_GetDeviceInfo(outParams.device)->name);

  outParams.channelCount = audioOptions.channelCount;

  if (outParams.channelCount > Pa_GetDeviceInfo(outParams.device)->maxOutputChannels) {
    err = std::string("Channel count exceeds maximum number of output channels for device");
    Napi::TypeError::New(env, err.c_str()).ThrowAsJavaScriptException();
  }

  outParams.sampleFormat = paFloat32;
  outParams.hostApiSpecificStreamInfo = NULL;
  outParams.suggestedLatency = Pa_GetDeviceInfo(outParams.device)->defaultLowOutputLatency;

  #ifdef __arm__
  outParams.suggestedLatency = Pa_GetDeviceInfo(outParams.device)->defaultHighOutputLatency;
  #endif

  const double sampleRate = (double) audioOptions.sampleRate;
  const int framesPerBuffer = audioOptions.framesPerBuffer;

  errCode = Pa_OpenStream(
    &paOutputStream,
    NULL,
    &outParams,
    sampleRate,
    framesPerBuffer,
    paNoFlag, // should be interleaved
    cb,
    NULL // user data
  );

  if (errCode != paNoError) {
    err = std::string("Could not open stream: ") + Pa_GetErrorText(errCode);
    Napi::TypeError::New(env, err.c_str()).ThrowAsJavaScriptException();
  }
}

void startPaOutputStream(Napi::Env env) {
  PaError errCode = Pa_StartStream(paOutputStream);

  std::cout << "startPaOutputStream()\t" << pthread_self() << std::endl;

  if (errCode != paNoError) {
    std::string err = std::string("Could not start output stream: ") + Pa_GetErrorText(errCode);
    Napi::TypeError::New(env, err.c_str()).ThrowAsJavaScriptException();
  }
}

void stopPaOutputStream(Napi::Env env) {
  Pa_StopStream(paOutputStream);
  Pa_Terminate();
}


// ----------------------------------------------------------------------
// CALLBACKS
// ----------------------------------------------------------------------

int paOutputCallback(
  const void *paInputBuffer,
  void *paOutputBuffer,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo *timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData
) {
  // @note - should lock things

  uint32_t requiredBufferIndex = outputBufferCounter;
  asyncHandle.data = (void *) &requiredBufferIndex;

  uv_async_send(&asyncHandle); // return immediately

  float* dest = (float*) paOutputBuffer;
  int length = (int) frameCount;

  uint32_t copyBufferIndex = (outputBufferCounter + 1) & 1;
  std::shared_ptr<std::vector<float>> src = asyncOuputData.buffers[copyBufferIndex];

  // populate output
  for (int i = 0; i < length; i++)
    dest[i] = (float) src->at(i);

  // update counter for next call
  outputBufferCounter = (outputBufferCounter + 1) & 1;

  // return ++counter < 20 ? paContinue : paComplete; // context->fillBuffer(output, frameCount) ? paContinue : paComplete;
  return paContinue;
}

void pullOutputCallback(uv_async_t* handle) {
  std::shared_ptr<Napi::Env> envPointer = asyncOuputData.env;
  Napi::Env env = *envPointer.get();
  Napi::HandleScope scope(env);
  napi_value global = env.Global();

  std::shared_ptr<AudioOptions> options = asyncOuputData.options;

  uint32_t requiredBufferIndex = *((uint32_t*) handle->data);
  std::shared_ptr<std::vector<float>> buffer = asyncOuputData.buffers[requiredBufferIndex];
  float* data = buffer->data();
  int size = buffer->size();

  Napi::ArrayBuffer arrayBuffer = Napi::ArrayBuffer::New(env, data, size * sizeof(float));
  Napi::Float32Array output = Napi::Float32Array::New(env, size, arrayBuffer, 0);
  Napi::Number framesPerBuffer = Napi::Number::New(env, options->framesPerBuffer);
  Napi::Number numChannels = Napi::Number::New(env, options->channelCount);

  persistentOutputCallback.MakeCallback(global, { output, framesPerBuffer, numChannels });
}


// ----------------------------------------------------------------------
// PUBLIC API
// ----------------------------------------------------------------------

int paOutputInitialized = 0;

/**
 * Configure output
 */
Napi::Value Configure(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  std::cout << "output::configure()\t" << pthread_self() << std::endl;

  if (info.Length() > 1) {
    std::string str = "Invalid number of arguments (0 or 1)";
    Napi::TypeError::New(env, str.c_str()).ThrowAsJavaScriptException();
    return env.Null();
  }

  // options
  Napi::Object objOptions;

  // @todo - handle options when called from Start();
  if (info.Length() == 1)
    objOptions = info[0].As<Napi::Object>();
  else
    objOptions = Napi::Object::New(env);

  AudioOptions options(objOptions);
  Napi::Value processValue = objOptions.Get("process");

  if (processValue.IsUndefined()) {
    std::string str = "Invalid process function";
    Napi::TypeError::New(env, str.c_str()).ThrowAsJavaScriptException();
    return env.Null();
  }

  // store things for later usage
  Napi::Function processFunction = processValue.As<Napi::Function>();
  persistentOutputCallback = Napi::Persistent(processFunction);
  asyncOuputData.env = std::make_shared<Napi::Env>(env);

  // @note - need some test
  #ifdef __arm__
  options.framesPerBuffer = 256;
  #endif

  // allocate buffers for double buffering
  const int bufferSize = options.framesPerBuffer * options.channelCount;

  for (int i = 0; i < 2; i++) {
    std::vector<float> buffer(bufferSize, 0);
    asyncOuputData.buffers[i] = std::make_shared<std::vector<float>>(buffer);
  }

  asyncOuputData.options = std::make_shared<AudioOptions>(options);

  // init portaudio
  initPaOutputStream(env, options, paOutputCallback);
  paOutputInitialized = 1;

  return Napi::Boolean::New(env, true);
}

void Start(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (paOutputInitialized == 0)
    Configure(info);

  uv_async_init(uv_default_loop(), &asyncHandle, pullOutputCallback);
  startPaOutputStream(env);

  return;
}

void Stop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  uv_close((uv_handle_t*) &asyncHandle, NULL);
  stopPaOutputStream(env);

  paOutputInitialized = 0;

  return;
}

Napi::Object CreateOutput(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object obj = Napi::Object::New(env);

  obj["configure"] = Napi::Function::New(env, Configure);
  obj["start"] = Napi::Function::New(env, Start);
  obj["stop"] = Napi::Function::New(env, Stop);

  return obj;
}

}; // namespace
