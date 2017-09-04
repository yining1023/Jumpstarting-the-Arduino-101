const serviceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214"; // lowercase hex characters e.g. '00001234-0000-1000-8000-00805f9b34fb'
const name = 'LEDCB';

var ledCharacteristic;

function connectTo101() {
  let options = {
    filters: [{
      services: [serviceUuid],
      name: name
    }]}

  console.log('Requesting Bluetooth Device...');

  navigator.bluetooth.requestDevice(options)
  .then(device => {
    console.log('Got device', device.name);
    return device.gatt.connect();
  })
  .then(server => {
      console.log('Getting Service...');
      return server.getPrimaryService(serviceUuid);
    })
    .then(service => {
      console.log('Getting Characteristics...');
      return service.getCharacteristics();
    })
    .then(characteristics => {
      console.log('Got LED Characteristic');
      ledCharacteristic = characteristics[0];
    })
    .catch(error => {
      console.log('Argh! ' + error);
    });
}

function writeTo101() {
  let inputValue = document.getElementById('input-box').value;

  let bufferToSend = Uint8Array.of(inputValue);
  ledCharacteristic.writeValue(bufferToSend);
  console.log('Writing '+ inputValue + ' to led Characteristic...');

}
