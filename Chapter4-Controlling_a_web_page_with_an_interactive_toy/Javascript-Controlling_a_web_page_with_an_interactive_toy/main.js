const serviceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214"; // lowercase hex characters e.g. '00001234-0000-1000-8000-00805f9b34fb'
const name = 'CurieTap';

var bluetoothDevice;
var tapCharacteristic;

var tapValue;
var heartImage, cryheartImage, ouch, ouchouch;
var scaleValue = 4;

function connect() {
  let options = {
    filters: [{
      services: [serviceUuid],
      name: name
    }]}

  console.log('Requesting Bluetooth Device...');

  navigator.bluetooth.requestDevice(options)
  .then(device => {
    bluetoothDevice = device; // save a copy
    console.log('Got device', device.name);
    return device.gatt.connect();
  })
  .then(server => {
      console.log('Getting Service...');
      return server.getPrimaryService(serviceUuid);
    })
    .then(service => {
      console.log('Getting Characteristics...');
      // Get all characteristics.
      return service.getCharacteristics();
    })
    .then(characteristics => {
      console.log('Got Characteristics');
      tapCharacteristic = characteristics[0];
      tapCharacteristic.addEventListener('characteristicvaluechanged', handleTap);
    })
    .catch(error => {
      console.log('Argh! ' + error);
    });
  }

function startNotify() {
  tapCharacteristic.startNotifications();
}

function handleTap(event) {
  tapValue = event.target.value.getUint8(0);
  console.log('tap: ' + tapValue);
  playSound();
  scaleImage();
}

function preload() {
  heartImage = loadImage("assets/heart.png");
  cryheartImage = loadImage("assets/cryheart.png");
  ouch = loadSound('assets/ouch.m4a');
  ouchouch = loadSound('assets/ouchouch.m4a');
}

function setup() {
  createCanvas(800, 700);
  imageMode(CENTER);
}

function draw() {
  background('#FFE3DF');

  if (scaleValue <= 4) {
    scaleValue = 4;
    image(heartImage, width / 2, height / 2, heartImage.width / scaleValue, heartImage.height / scaleValue);
  } else {
    image(cryheartImage, width / 2, height / 2, heartImage.width / scaleValue, heartImage.height / scaleValue);
    scaleValue -= 0.2;
  }
}

function scaleImage() {
  if (tapValue === 1) {
    scaleValue = 8;
  } else if (tapValue === 2) {
    scaleValue = 10;
  } else {
    scaleValue = 4;
  }
}

function playSound() {
  if (tapValue === 1) {
    ouch.setVolume(0.5);
    ouchouch.stop();
    if (ouch.isPlaying()) ouch.stop();
    ouch.play();
  } else if (tapValue === 2) {
    ouchouch.setVolume(0.5);
    ouch.stop();
    if (ouchouch.isPlaying()) ouchouch.stop();
    ouchouch.play();
  }
}

function disconnect() {
  if (bluetoothDevice && bluetoothDevice.gatt) {
    bluetoothDevice.gatt.disconnect();
    console.log('Discoonected');
  }
}
