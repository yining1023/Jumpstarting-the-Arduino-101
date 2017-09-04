int ledPin = 13;               // LED connected to digital pin 13

void setup() {                 // setup() function is called when a sketch starts
  pinMode(ledPin, OUTPUT);     // sets the digital pin as output
}
void loop() {                  // loop() function will be called over and over again
  digitalWrite(ledPin, HIGH);  // sets the LED on
  delay(1000);                 // waits for a second
  digitalWrite(ledPin, LOW);   // sets the LED off
  delay(1000);                 // waits for a second
}
