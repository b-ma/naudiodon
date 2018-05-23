# `node-portaudio`

> Node.js wrapper around [PortAudio](http://portaudio.com/). 

This library is a (messy) fork of [naudiodon](/Streampunk/naudiodon) (itself a fork of [node-portaudio](/joeferner/node-portaudio)), with the following changes:

- Changing the stream model to a pull model with double buffering, this change intends to provide a more responsive environment and a better clock stability for real-time usages;
- Updating from [Nan](/nodejs/nan) to [n-api c++](https://nodejs.org/api/n-api.html) API.

_Unless you really need this features (aka most of the time) please use [naudiodon](/Streampunk/naudiodon) or similar stream-based library_

## Installation

```
npm install [--save] @b-ma/node-portaudio
```

## Example

```js
const portaudio = require('@bma/node-portaudio');

// retrieve the list of devices
console.log(portaudio.getDevices());

// do some synthesis
const frequency = 400;
const sampleRate = 44100;
let phase = 0;

function sine(output, frameSize, channelCount) {
  for (let i = 0; i < frameSize; i++) {
    output[i] = Math.sin(phase * Math.PI * 2);
    phase += frequency / sampleRate;

    if (phase >= 1)
      phase -= 1;
  }
}

portaudio.output.configure({
  sampleRate: sampleRate,
  channelCount: 1,
  process: (output, framesPerBuffer, channelCount) => {
    sine(output, framesPerBuffer, channelCount);
  }
});

portaudio.output.start();

setTimeout(() => portaudio.output.stop(), 3000);


```

## @todo

- figure out what happens with stereo output
- implement non interleaved mode
- implement input

## License

BSD-3-Clause
