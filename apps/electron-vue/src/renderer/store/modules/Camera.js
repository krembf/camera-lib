var addon = require('bindings')({bindings: 'cameraaddon.node', module_root: '../../'})

var getCameraName = function () {
  return addon.GetCameraName()
}

var snap = function () {
  return addon.Snap()
}

var snapAsync = function (buffer, cb) {
  return addon.snapAsync(buffer, cb)
}

var snapEmit = function (buffer, emitter) {
  addon.snapEmit(buffer, emitter)
}

var snapEmitThread = async function (buffer, emitter) {
  await addon.snapEmitThread(buffer, emitter)
}

export default {
  getCameraName,
  snap,
  snapAsync,
  snapEmit,
  snapEmitThread
}
