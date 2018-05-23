const portAudioBindings = require('bindings')('node-portaudio.node');

const exportObject = {
  getDevices: portAudioBindings.getDevices,
  output: portAudioBindings.createOutput(),
};

module.exports = exportObject;
