#include <CurieBLE.h>
#include "CurieIMU.h"

BLEService tapService("19B10000-E8F2-537E-4F6C-D104768A1214"); // create service

// create single tap characteristic and allow remote device to read and write
BLEUnsignedCharCharacteristic tapCharacteristic("5667f3b1-d6a2-4fb2-a917-4bee580a9c84", BLERead | BLENotify);

int tapValue = 0;
void setup() {
  Serial.begin(9600); // initialize Serial communication
  while(!Serial) ;    // wait for serial port to connect.
  // Initialise the IMU
  CurieIMU.begin();
  CurieIMU.attachInterrupt(eventCallback);

  // Increase Accelerometer range to allow detection of stronger taps (< 4g)
  CurieIMU.setAccelerometerRange(4);

  // Reduce threshold to allow detection of weaker taps (>= 750mg)
  CurieIMU.setDetectionThreshold(CURIE_IMU_TAP, 750); // (750mg)

  // Reduce threshold to allow detection of weaker taps (>= 750mg)
  CurieIMU.setDetectionThreshold(CURIE_IMU_DOUBLE_TAP, 750); // (750mg)

  // Set the quite time window for 2 taps to be registered as a double-tap (Gap time between taps <= 1000 milliseconds)
  CurieIMU.setDetectionDuration(CURIE_IMU_DOUBLE_TAP, 1000);

  // Enable Tap detection
  CurieIMU.interrupts(CURIE_IMU_TAP);

  // Enable Double-Tap detection
  CurieIMU.interrupts(CURIE_IMU_DOUBLE_TAP);

  Serial.println("IMU initialization complete, waiting for events...");

  // begin initialization
  BLE.begin();

  // set the local name peripheral advertises
  BLE.setLocalName("CurieTap");
  // set the UUID for the service this peripheral advertises
  BLE.setAdvertisedService(tapService);

  // add the characteristic to the service
  tapService.addCharacteristic(tapCharacteristic);

  // add service
  BLE.addService(tapService);

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // set an initial value for the characteristic
  tapCharacteristic.setValue(tapValue);

  // start advertising
  BLE.advertise();

  Serial.println(("Bluetooth device active, waiting for connections..."));
}

void loop() {
  // poll for BLE events
  BLE.poll();
  
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      tapCharacteristic.setValue(tapValue);
    }
  }
  
}

static void eventCallback()
{
  if (CurieIMU.getInterruptStatus(CURIE_IMU_DOUBLE_TAP)) {
    Serial.println("DOUBLE Tap");
    tapValue = 2;
  } else if (CurieIMU.getInterruptStatus(CURIE_IMU_TAP)) {
    Serial.println("SINGLE Tap");
    tapValue = 1;
  } else {
    tapValue = 0;
  }
}

void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}

