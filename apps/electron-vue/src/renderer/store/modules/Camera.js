var addon = require('bindings')({bindings: 'cameraaddon.node', module_root: '../../'})

var getCameraName = function () {
  return addon.GetCameraName()
}

var snap = function () {
  return addon.Snap()
}

export default {
  getCameraName,
  snap
}
