var gateway = `ws://${window.location.hostname}/ws`
var websocket

window.addEventListener('load', onLoad)

function initWebSocket() {
  console.log('Trying to open a WebSocket connection...')
  websocket = new WebSocket(gateway)
  websocket.onopen = onOpen
  websocket.onclose = onClose
  websocket.onmessage = onMessage // <-- add this line
}

function onOpen(event) {
  console.log('Connection opened')
}
function onClose(event) {
  console.log('Connection closed')
  setTimeout(initWebSocket, 2000)
}
function onMessage(event) {
  let data = JSON.parse(event.data)
  console.log(data)
  document.getElementById('content').innerHTML = state
  // document.getElementById('drill').innerHTML = data.status
  // document.getElementById('retract').innerHTML = data.status
  // document.getElementById('turn').innerHTML = data.status
  // document.getElementById('auto').innerHTML = data.status
}
function onLoad(event) {
  initWebSocket()
  initButton()
}

function initButton() {
  document.getElementById('drill').addEventListener('click', drill)
  document.getElementById('retract').addEventListener('click', retract)
  document.getElementById('turn').addEventListener('click', turn)
  document.getElementById('auto').addEventListener('click', auto)
}
function drill() {
  websocket.send('drill')
}
function retract() {
  websocket.send('retract')
}
function turn() {
  websocket.send('turn')
}
function auto() {
  websocket.send('auto')
}
// const handleChangeEffect = async() => {
//   //fetch('http://192.168.0.165/Rainbow');
//   const data = await axios.get('http://192.168.0.165/Rainbow')
//   console.log(data)
//  }
