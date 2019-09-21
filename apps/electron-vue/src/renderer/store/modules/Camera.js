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

export default {
  getCameraName,
  snap,
  snapAsync
}
