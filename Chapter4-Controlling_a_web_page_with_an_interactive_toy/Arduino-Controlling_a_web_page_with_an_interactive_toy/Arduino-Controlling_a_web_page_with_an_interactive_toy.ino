#include <CurieBLE.h>
#include "CurieIMU.h"

BLEPeripheral blePeripheral;
BLEService tapService = BLEService("19B10000-E8F2-537E-4F6C-D104768A1214"); // create service

// create single tap characteristic and allow remote device to read and write
BLEUnsignedCharCharacteristic tapCharacteristic = BLEUnsignedCharCharacteristic("5667f3b1-d6a2-4fb2-a917-4bee580a9c84", BLERead | BLENotify);

int tapValue = 0;

void setup() {
  Serial.begin(9600); // initialize Serial communication

  // set the local name peripheral advertises
  blePeripheral.setLocalName("CurieTap");
  // set the UUID for the service this peripheral advertises
  blePeripheral.setAdvertisedServiceUuid(tapService.uuid());

  // add service and characteristic
  blePeripheral.addAttribute(tapService);
  blePeripheral.addAttribute(tapCharacteristic);

  blePeripheral.begin();

  tapCharacteristic.setValue(tapValue);
  
  Serial.println(("Bluetooth device active, waiting for connections..."));

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
}

void loop() {
  // Tell the bluetooth radio to do whatever it should be working on
  blePeripheral.poll();
}

static void eventCallback()
{
  if (CurieIMU.getInterruptStatus(CURIE_IMU_DOUBLE_TAP)) {
    Serial.println("DOUBLE Tap");
    tapValue = 2;
    tapCharacteristic.setValue(tapValue);
  } else if (CurieIMU.getInterruptStatus(CURIE_IMU_TAP)) {
    Serial.println("SINGLE Tap");
    tapValue = 1;
    tapCharacteristic.setValue(tapValue);
  }
}

