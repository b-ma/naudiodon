const portaudio = require('..');

let counter = 0;

const frequency = 400;
const sampleRate = 44100;
let phase = 0;
const dPhase = frequency / sampleRate;
const _2PI = Math.PI * 2;

function sine(output, frameSize, channelCount) {
  for (let i = 0; i < frameSize; i++) {
    const value = Math.sin(phase * _2PI);
    output[i] = value;

    phase += dPhase;

    if (phase >= 1)
      phase -= 1;
  }
}

portaudio.output.configure({
  sampleRate: sampleRate,
  // deviceId: 3,
  channelCount: 1,
  process: (output, framesPerBuffer, channelCount) => {
    sine(output, framesPerBuffer, channelCount);
  }
});

console.log(portaudio.output)

portaudio.output.start();

setTimeout(() => {
  portaudio.output.stop();

  // setTimeout(() => {
  //   portaudio.output.start();

  //   setTimeout(() => {
  //     portaudio.output.stop();
  //   }, 1000);
  // }, 1000);
}, 3000);
