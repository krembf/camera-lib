var chai = require('chai')
const EventEmitter = require('events').EventEmitter

var addon = require('bindings')({ bindings: 'cameraaddon.node', module_root: '../../' })

// Local variales
var expect = chai.expect

describe('addon', function () {
	// describe('callEmit', function () {
	// 	it('should emit events', function () {
	// 		const emitter = new EventEmitter()
	// 		sensor1 = 0
	// 		sensor2 = 0

	// 		emitter.on('start', () => {
	// 			console.log('### Sensor reading started ...');
	// 		})

	// 		emitter.on('sensor1', (evt) => {
	// 			// This module will be called as on when the
	// 			// sensor1 data available for consuming from JS
	// 			console.log(evt);
	// 			expect(evt).to.equal(`sensor1 data ${++sensor1} ...`)
	// 		})

	// 		emitter.on('sensor2', (evt) => {
	// 			console.log(evt);
	// 			expect(evt).to.equal(`sensor2 data ${++sensor2} ...`)
	// 		})

	// 		emitter.on('end', () => {
	// 			console.log('### Sensor reading Ended');
	// 		})

	// 		addon.callEmit(emitter.emit.bind(emitter))
	// 	})
	// })
	// describe('snapEmit', function () {
	// 	it('should emit events', function (done) {
	// 		const emitter = new EventEmitter()

	// 		emitter.on('start', () => {
	// 			console.log('### Sensor reading started ...');
	// 		})

	// 		emitter.on('buffer', (buffer) => {
	// 			// This module will be called as on when the
	// 			// sensor1 data available for consuming from JS
	// 			console.log(`got buffer length ${buffer.length}, first pixel value ${buffer[0]}`);
	// 			expect(buffer.length).to.equal(2304000)
	// 		})

	// 		emitter.on('end', function () {
	// 			console.log('### Sensor reading Ended');
	// 			done()
	// 		})

	// 		addon.snapEmit(emitter.emit.bind(emitter))
	// 	})
	// })
	describe('snapEmitThread', function () {
		it('should emit events', function (done) {
			let buffer = new Uint8Array(2304000)
			const emitter = new EventEmitter()

			emitter.on('start', () => {
				console.log('### Sensor reading started ...');
			})

			emitter.on('buffer', (buffer) => {
				// This module will be called as on when the
				// sensor1 data available for consuming from JS
				console.log(`got buffer length ${buffer.length}, first pixel value ${buffer[0]}`);
				expect(buffer.length).to.equal(2304000)
			})

			emitter.on('end', function () {
				console.log('### Sensor reading Ended');
				done()
			})

			addon.snapEmitThread(buffer, emitter.emit.bind(emitter))
		})
	})	
})