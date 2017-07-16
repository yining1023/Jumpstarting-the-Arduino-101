/*
 * This example demonstrates using the pattern matching engine (CuriePME)
 * to classify streams of accelerometer data from CurieIMU.
 *
 * It is based on the DrawingInTheAir Example by Intel Corporation.
 * 
 */

#include "CurieIMU.h"
#include "CuriePME.h"
#include <CurieBLE.h>

BLEPeripheral blePeripheral;
BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1216");

BLEUnsignedCharCharacteristic CharacteristicPattern(
  "4227f3b1-d6a2-4fb2-a916-3bee580a9c84", 
  BLERead | BLENotify
);

/*  This controls how many times a letter must be drawn during training.
 *  Any higher than 4, and you may not have enough neurons for all 26 letters
 *  of the alphabet. Lower than 4 means less work for you to train a letter,
 *  but the PME may have a harder time classifying that letter. */
const unsigned int trainingReps = 4;

/* Increase this to 'A-Z' if you like-- it just takes a lot longer to train */
const unsigned char trainingStart = 'A';
const unsigned char trainingEnd = 'B';

/* The input pin used to signal when a letter is being drawn- you'll
 * need to make sure a button is attached to this pin */
const unsigned int buttonPin = 4;
const int ledPin = 13;

/* Sample rate for accelerometer */
const unsigned int sampleRateHZ = 200;

/* No. of bytes that one neuron can hold */
const unsigned int vectorNumBytes = 128;

/* Number of processed samples (1 sample == accel x, y, z)
 * that can fit inside a neuron */
const unsigned int samplesPerVector = (vectorNumBytes / 3);

/* This value is used to convert ASCII characters A-Z
 * into decimal values 1-26, and back again. */
const unsigned int upperStart = 0x40;

const unsigned int sensorBufSize = 2048;
const int IMULow = -32768;
const int IMUHigh = 32767;

byte vector[vectorNumBytes];
unsigned int category;
char letter;
char prevLetter;

void setup()
{
  Serial.begin(9600);

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // setup ble
  blePeripheral.setLocalName("CuriePME");
  blePeripheral.setAdvertisedServiceUuid(customService.uuid());
  blePeripheral.addAttribute(customService);
  blePeripheral.addAttribute(CharacteristicPattern);

  CharacteristicPattern.setValue('0');
  blePeripheral.begin();

  /* Start the IMU (Intertial Measurement Unit), enable the accelerometer */
  CurieIMU.begin(ACCEL);

  /* Start the PME (Pattern Matching Engine) */
  CuriePME.begin();

  CurieIMU.setAccelerometerRate(sampleRateHZ);
  CurieIMU.setAccelerometerRange(2);

  while(!Serial);
  beginTraining();
  Serial.println("Training complete. Now, make a gesture (remember to ");
  Serial.println("hold the button) and see if the PME can classify it.");
  Serial.println("Use another BLE device to connect to Arduino101 first");
}

void loop ()
{
  BLECentral central = blePeripheral.central();

  // turn off LED by default
  digitalWrite(ledPin, LOW);

  // if button is pressed, read and classify the
  // accelerometer vector from the IMU
  if(digitalRead(buttonPin) == HIGH) {
    readVectorFromIMU(vector);
    classify(vector);
  }

  // if a change in the gesture or letter is 
  // detected, update the characteristic
  if (prevLetter != letter) {
    if(central) {
      if (central.connected()) {
        CharacteristicPattern.setValue(letter); 
        prevLetter = letter;
      }
    }
  }
}

/* Simple "moving average" filter, removes low noise and other small
 * anomalies, with the effect of smoothing out the data stream. */
byte getAverageSample(byte samples[], unsigned int num, unsigned int pos,
           unsigned int step)
{
  unsigned int ret;
  unsigned int size = step * 2;

  if (pos < (step * 3) || pos > (num * 3) - (step * 3)) {
    ret = samples[pos];
  } else {
    ret = 0;
    pos -= (step * 3);
    for (unsigned int i = 0; i < size; ++i) {
      ret += samples[pos - (3 * i)];
    }

    ret /= size;
  }

  return (byte)ret;
}

/* We need to compress the stream of raw accelerometer data into 128 bytes, so
 * it will fit into a neuron, while preserving as much of the original pattern
 * as possible. Assuming there will typically be 1-2 seconds worth of
 * accelerometer data at 200Hz, we will need to throw away over 90% of it to
 * meet that goal!
 *
 * This is done in 2 ways:
 *
 * 1. Each sample consists of 3 signed 16-bit values (one each for X, Y and Z).
 *    Map each 16 bit value to a range of 0-255 and pack it into a byte,
 *    cutting sample size in half.
 *
 * 2. Undersample. If we are sampling at 200Hz and the button is held for 1.2
 *    seconds, then we'll have ~240 samples. Since we know now that each
 *    sample, once compressed, will occupy 3 of our neuron's 128 bytes
 *    (see #1), then we know we can only fit 42 of those 240 samples into a
 *    single neuron (128 / 3 = 42.666). So if we take (for example) every 5th
 *    sample until we have 42, then we should cover most of the sample window
 *    and have some semblance of the original pattern. */
void undersample(byte samples[], int numSamples, byte vector[])
{
  unsigned int vi = 0;
  unsigned int si = 0;
  unsigned int step = numSamples / samplesPerVector;
  unsigned int remainder = numSamples - (step * samplesPerVector);

  /* Centre sample window */
  samples += (remainder / 2) * 3;
  for (unsigned int i = 0; i < samplesPerVector; ++i) {
    for (unsigned int j = 0; j < 3; ++j) {
      vector[vi + j] = getAverageSample(samples, numSamples, si + j, step);
    }

    si += (step * 3);
    vi += 3;
  }
}

void beginTraining()
{
  for (char i = trainingStart; i <= trainingEnd; ++i) {
    Serial.print("Hold down the button and make a gesture or '");
    Serial.print(String(i) + "' in the air. Release the button as soon ");
    Serial.println("as you are done.");

    trainGesture(i, trainingReps);
    Serial.println("OK, finished with this gesture.");
    delay(2000);
  }
}

void trainGesture(char letter, unsigned int repeat)
{
  unsigned int i = 0;

  while (i < repeat) {
    byte vector[vectorNumBytes];

    if (i) Serial.println("And again...");

    readVectorFromIMU(vector);
    CuriePME.learn(vector, vectorNumBytes, letter - upperStart);

    Serial.println("Got it!");
    delay(1000);
    ++i;
  }
}

void readVectorFromIMU(byte vector[])
{
  byte accel[sensorBufSize];
  int raw[3];

  unsigned int samples = 0;
  unsigned int i = 0;

  /* Wait until button is pressed */
  while (digitalRead(buttonPin) == LOW) {
    digitalWrite(ledPin, LOW);
  }

  /* While button is being held... */
  while (digitalRead(buttonPin) == HIGH) {
    digitalWrite(ledPin, HIGH);
    if (CurieIMU.dataReady()) {
      CurieIMU.readAccelerometer(raw[0], raw[1], raw[2]);

      /* Map raw values to 0-255 */
      accel[i] = (byte) map(raw[0], IMULow, IMUHigh, 0, 255);
      accel[i + 1] = (byte) map(raw[1], IMULow, IMUHigh, 0, 255);
      accel[i + 2] = (byte) map(raw[2], IMULow, IMUHigh, 0, 255);

      i += 3;
      ++samples;

      /* If there's not enough room left in the buffers
      * for the next read, then we're done */
      if (i + 3 > sensorBufSize) {
        break;
      }
    }
  }
  undersample(accel, samples, vector);
}

void classify(byte vector[])
{
  /* Use the PME to classify the vector, i.e. return a category from 1-26, representing a letter from A-Z */
  category = CuriePME.classify(vector, vectorNumBytes);
  Serial.println("get category: ");
  Serial.println(category);
  prevLetter = letter;
  if (category == CuriePME.noMatch) {
    Serial.println("Don't recognise that one-- try again.");
    letter = '0';
  } else {
    letter = category + upperStart;
    Serial.println("The gesture is:");
    Serial.println(letter);
  }
}

