var addon = require('bindings')({bindings: 'cameraaddon.node', module_root: '../../'})

var getCameraName = function () {
  return addon.GetCameraName()
}

var snap = function () {
  return addon.Snap()
}

var snapAsync = function (buffer, cb) {
  return addon.SnapAsync(buffer, cb, 200)
}

var snapWithCallback = function (buffer, cb) {
  return addon.SnapWithCallback(buffer, cb, 200)
}

export default {
  getCameraName,
  snap,
  snapAsync,
  snapWithCallback
}
