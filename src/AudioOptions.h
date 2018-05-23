#pragma once

#include "portaudio.h"

namespace bma {

class AudioOptions {
  public:
    AudioOptions(Napi::Object obj)
      : deviceId(unpackNum(obj, "deviceId", paNoDevice))
      , sampleRate(unpackNum(obj, "sampleRate", 44100))
      , channelCount(unpackNum(obj, "channelCount", 2))
      , framesPerBuffer(unpackNum(obj, "framesPerBuffer", 128))
    {}
    ~AudioOptions() {}

    int deviceId;
    int sampleRate;
    int channelCount;
    int sampleFormat;
    int framesPerBuffer;

    void log(std::string type) const  {

      std::cout << "---------------------------------------------" << std::endl;
      if (deviceId == Pa_GetDefaultOutputDevice())
        std::cout << "| Output (default): " << Pa_GetDeviceInfo(deviceId)->name << std::endl;
      else
        std::cout << "| Output: " << Pa_GetDeviceInfo(deviceId)->name << std::endl;
      std::cout << "---------------------------------------------" << std::endl;
      std::cout << "| - deviceId: " << deviceId << std::endl;
      std::cout << "| - sampleRate: " << sampleRate << std::endl;
      std::cout << "| - channelCount: " << channelCount << std::endl;
      std::cout << "| - framesPerBuffer: " << framesPerBuffer << std::endl;
      std::cout << "---------------------------------------------" << std::endl;

      // return ss.str();
    }

  private:
    int unpackNum(Napi::Object obj, const std::string& key, int defaultValue) {
      int result = defaultValue;
      Napi::Value val = obj.Get(key);

      if (!val.IsUndefined()) {
        result = (int) val.ToNumber();
      }

      return result;
    }
};

} // namespace bma

