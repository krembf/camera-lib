// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.
console.log("Renderer Enter");

const SerialPort = require('serialport')
const MockBinding = require('@serialport/binding-mock')
const Readline = require('@serialport/parser-readline')
const path = '/dev/ROBOT';
SerialPort.Binding = MockBinding
// Create a port and enable the echo and recording.
MockBinding.createPort(path, { echo: true, record: true, readyData: "FREDDY" })
const port = new SerialPort(path)


var addon = require('bindings')('cameraaddon.node');

document.body.onload = addElement;

function addElement () { 
  // create a new div element 
  var newDiv = document.createElement("div"); 
  // and give it some content 
  var newContent = document.createTextNode(addon.hello()); 
  // add the text node to the newly created div
  newDiv.appendChild(newContent);  

  // add the newly created element and its content into the DOM 
  var currentDiv = document.getElementById("div1"); 
  document.body.insertBefore(newDiv, currentDiv); 

  const parser = new Readline()
  port.pipe(parser)

  parser.on('data', line => console.log(`> ${line}`))
  port.write('ROBOT POWER ON\n')
}

function getData() {
  console.log("getData clicked");
}

let getData2 = function() {
  console.log("getData clicked");
}