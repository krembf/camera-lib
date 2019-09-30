<template>
  <div id="wrapper">
    <img id="logo" src="~@/assets/logo.png" alt="electron-vue" />
    <main>
      <div class="left-side">
        <span class="title">Welcome to your new project!</span>
        <canvas id="canvas" width="960" height="600"></canvas>
      </div>

      <div class="right-side">
        <div class="doc">
          <div class="title">Getting Started</div>
          <p>
            electron-vue comes packed with detailed documentation that covers everything from
            internal configurations, using the project structure, building your application,
            and so much more.
          </p>
          <button
            @click="open('https://simulatedgreg.gitbooks.io/electron-vue/content/')"
          >Read the Docs</button>
          <br />
          <br />
        </div>
        <div class="doc">
          <div class="title alt">Other Documentation</div>
          <button class="alt" @click="open('https://electron.atom.io/docs/')">Electron</button>
          <button class="alt" @click="open('https://vuejs.org/v2/guide/')">Vue.js</button>
          <button class="alt" @click="getCameraName('Hi')">My New Button</button>
          <button class="alt" @click="snapAsync()">Snap</button>
        </div>
      </div>
    </main>
  </div>
</template>

<script>
import SystemInformation from './LandingPage/SystemInformation'
import Camera from '../store/modules/Camera'

export default {
  name: 'landing-page',
  components: { SystemInformation },
  methods: {
    open (link) {
      this.$electron.shell.openExternal(link)
    },
    getCameraName (message) {
      alert(message + Camera.getCameraName())
    },
    snap () {
      let buffer = Camera.snap()
      var canvas = document.querySelector('canvas')
      var ctx = canvas.getContext('2d')
      var imgData = ctx.createImageData(960, 600)
      let j = 0
      buffer.forEach(element => {
        imgData.data[j] = element
        j++
      })
      ctx.putImageData(imgData, 0, 0)
    },
    snapAsync () {
      let bufferSize = 2304000
      let numberOfBuffers = 4
      let buffer = new Uint8Array(bufferSize * numberOfBuffers)
      let bufferIndex = 0
      // main canvas
      let canvas = document.querySelector('canvas')
      let ctx = canvas.getContext('2d', { alpha: false })
      // buffer canvas
      let canvas2 = document.createElement('canvas')
      canvas2.width = 960
      canvas2.height = 600
      let context2 = canvas2.getContext('2d', { alpha: false })
      let imgData = context2.createImageData(960, 600)
      Camera.snapWithCallback(buffer, function () {
        console.log('### Got new buffer')
        // create something on the canvas
        let j = 0
        var start = Date.now()
        buffer.subarray(bufferIndex * bufferSize, bufferIndex * bufferSize + bufferSize).forEach(element => {
          imgData.data[j] = element
          j++
        })
        context2.putImageData(imgData, 0, 0)
        // render the buffered canvas onto the original canvas element
        ctx.drawImage(canvas2, 0, 0)
        bufferIndex = (bufferIndex + 1) % numberOfBuffers
        console.log(`Render took ${Date.now() - start} millis`)
      })
    }
  }
}
</script>

<style>
@import url("https://fonts.googleapis.com/css?family=Source+Sans+Pro");

* {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

body {
  font-family: "Source Sans Pro", sans-serif;
}

#wrapper {
  background: radial-gradient(
    ellipse at top left,
    rgba(255, 255, 255, 1) 40%,
    rgba(229, 229, 229, 0.9) 100%
  );
  height: 100vh;
  padding: 60px 80px;
  width: 100vw;
}

#logo {
  height: auto;
  margin-bottom: 20px;
  width: 420px;
}

main {
  display: flex;
  justify-content: space-between;
}

main > div {
  flex-basis: 50%;
}

.left-side {
  display: flex;
  flex-direction: column;
}

.welcome {
  color: #555;
  font-size: 23px;
  margin-bottom: 10px;
}

.title {
  color: #2c3e50;
  font-size: 20px;
  font-weight: bold;
  margin-bottom: 6px;
}

.title.alt {
  font-size: 18px;
  margin-bottom: 10px;
}

.doc p {
  color: black;
  margin-bottom: 10px;
}

.doc button {
  font-size: 0.8em;
  cursor: pointer;
  outline: none;
  padding: 0.75em 2em;
  border-radius: 2em;
  display: inline-block;
  color: #fff;
  background-color: #4fc08d;
  transition: all 0.15s ease;
  box-sizing: border-box;
  border: 1px solid #4fc08d;
}

.doc button.alt {
  color: #42b983;
  background-color: transparent;
}
</style>
