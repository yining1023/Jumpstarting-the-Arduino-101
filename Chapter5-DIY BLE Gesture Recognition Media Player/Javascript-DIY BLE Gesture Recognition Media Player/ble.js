// our service UUID and name
const serviceUuid = '19b10000-e8f2-537e-4f6c-d104768a1216';
const name = 'CuriePME';

var bluetoothDevice, characteristicPattern, value;
var pattern = '';

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
      console.log('Starting Notifications...');

      // select the first characteristic
      characteristicPattern = characteristics[0];

      // add event listener to handle characteristic value change
      characteristicPattern.addEventListener('characteristicvaluechanged', handleData);

      // finally start notifications on this characteristic
      return characteristicPattern.startNotifications();
    })
    .catch(error => {
      console.log('Argh! ' + error);
    });
  }

function handleData(event) {
  value = event.target.value.getUint8(0);
  console.log('> Got Pattern data: ' + value);
  if (value !== 48) {
    pattern = String.fromCharCode(value);
    console.log('Received Pattern ' + pattern);
  } else {
    pattern = null;
    console.log('Could not recognize the gesture...');
  }
}

function disconnect() {
  if (bluetoothDevice && bluetoothDevice.gatt) {
    bluetoothDevice.gatt.disconnect();
    console.log('Disconnected');
  }
}
