# `node-portaudio`

> Node.js wrapper around [PortAudio](http://portaudio.com/). 

This library is a (lot more messy) fork of [naudiodon](https://github.com//Streampunk/naudiodon) (itself a fork of [node-portaudio](https://github.com/joeferner/node-portaudio)), with the following modifications:

- Changing the stream model to a pull model with double buffering, this change intends to provide a more responsive environment and a better clock stability for real-time usages;
- Updating from [Nan](/nodejs/nan) to [N-API](https://nodejs.org/api/n-api.html) API.

_Unless you really need this features (aka most of the time) please use [naudiodon](https://github.com//Streampunk/naudiodon) or similar stream-based library_

## Installation

**warning**: not on `npm` yet, install from `github`: 

```
npm install [--save] b-ma/node-portaudio
```

## Example

```js
const portaudio = require('node-portaudio');

// retrieve the list of devices
console.log(portaudio.getDevices());

// do some synthesis
const frequency = 400;
const sampleRate = 44100;
let phase = 0;

function sine(output, frameSize, channelCount) {
  for (let i = 0; i < frameSize; i++) {
    output[i] = Math.sin(phase * Math.PI * 2);
    phase = (phase + frequency / sampleRate) % 1;
  }
}

portaudio.output.configure({
  sampleRate: sampleRate,
  channelCount: 1,
  process: (output, framesPerBuffer, channelCount) => {
    sine(output, framesPerBuffer, channelCount);
  }
});
// start audio rendering
portaudio.output.start();
// stop audio rendering
setTimeout(() => portaudio.output.stop(), 3000);
```

## @todo

- figure out what happens with stereo output
- implement non interleaved mode
- implement input

## License

BSD-3-Clause
