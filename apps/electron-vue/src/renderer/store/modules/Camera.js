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

var snapWithCallback = function (cammeraSettings, buffer, cb) {
  let wrappedCameraSettings = new addon.WrappedCameraSettings()
  wrappedCameraSettings.value = 42
  wrappedCameraSettings.imageWidth = cammeraSettings.imageWidth
  wrappedCameraSettings.imageHeight = cammeraSettings.imageHeight
  wrappedCameraSettings.byteDepth = cammeraSettings.byteDepth
  return addon.SnapWithCallback(buffer, cb, 200, wrappedCameraSettings)
}

export default {
  getCameraName,
  snap,
  snapAsync,
  snapWithCallback
}
