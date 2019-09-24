// Userland modules
var chai = require('chai')

// Local modules
// var addon = require('../')
var addon = require('bindings')({bindings: 'cameraaddon.node', module_root: '../../'})

// Local variales
var expect = chai.expect

describe('addon', function () {
  it('should have all expected keys', function () {
    expect(addon).to.contain.all.keys(['GetCameraName', 'Snap', 'SnapAsync'])
  })

  describe('.GetCameraName', function () {
    it('should be a function', function () {
      expect(addon.GetCameraName).to.be.a('function')
    })

    it('should return camera name', function () {
      let cameraName = addon.GetCameraName()
      expect(cameraName).to.equal('Basler')
    })
  });

  describe('.Snap', function () {
    it('should be a function', function () {
      expect(addon.Snap).to.be.a('function')
    })

    it('should return image buffer', function () {
      let buffer = addon.Snap()
      expect(buffer.length).to.equal(2304000)
    })
  });
  
  describe('.SnapAsync', function () {
    it('should be a function', function () {
      expect(addon.SnapAsync).to.be.a('function')
    })

    it('should return image buffer', function () {
      let bufferSize = 2304000;
      let numberOfBuffers = 4;
      let buffer = new Uint8Array(bufferSize * numberOfBuffers);
      let bufferIndex = 0;
      expect(buffer[0]).to.equal(0);
      addon.SnapAsync(buffer, function (cnt) {
        // expect(err).to.not.exist
        console.log(`buffer ${bufferIndex} First pixel value after snap ${buffer[bufferSize * bufferIndex]}`);
        expect(buffer[bufferSize * bufferIndex]).to.not.equal(0);
        bufferIndex = (bufferIndex + 1) % numberOfBuffers;
      }, 10)
    })     
  });
})
