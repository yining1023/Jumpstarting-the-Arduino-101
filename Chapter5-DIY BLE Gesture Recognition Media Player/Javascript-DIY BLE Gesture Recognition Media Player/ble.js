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
      characteristicPattern = characteristics[0];
      characteristicPattern.addEventListener('characteristicvaluechanged', handleData);

    })
    .catch(error => {
      console.log('Argh! ' + error);
    });
  }

function startNotify() {
  characteristicPattern.startNotifications();
}

function handleData(event) {
  value = event.target.value.getUint8(0);
  console.log('> Got Pattern data: ' + value);
  if (value !== 48) {
    letter = String.fromCharCode(value);
    console.log('Converted data to letter: : ' + letter);
  } else {
    letter = 'Waiting...';
    console.log('Could not recognize the gesture...');
  }
}

function disconnect() {
  if (bluetoothDevice && bluetoothDevice.gatt) {
    bluetoothDevice.gatt.disconnect();
    console.log('Discoonected');
  }
}