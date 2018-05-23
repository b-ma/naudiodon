#pragma once

#include <sstream>
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

    std::string toString(std::string type) const  {
      std::stringstream ss;

      ss << "-----------------------------" << std::endl;
      ss << type << " options: " << std::endl;
      if (deviceId == paNoDevice)
        ss << "- device: default" << std::endl;
      else
        ss << "- device: " << deviceId << std::endl;
      ss << "- sampleRate: " << sampleRate << std::endl;
      ss << "- channelCount: " << channelCount << std::endl;
      ss << "- framesPerBuffer: " << framesPerBuffer << std::endl;
      ss << "-----------------------------" << std::endl;

      return ss.str();
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

