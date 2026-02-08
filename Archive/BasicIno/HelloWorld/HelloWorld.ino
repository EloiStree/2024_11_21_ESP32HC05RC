#include <Arduino.h>
// Pin where the LED is connected
const int ledPin = 2; // ESP 32 S3

// Blink interval (milliseconds)
const unsigned long interval = 1000; 

// State variables
unsigned long previousMillis = 0;
bool ledState = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600); // Start serial communication at 9600 baud

      Serial.println("Hello World");
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to toggle the LED
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Toggle LED state
    ledState = !ledState;
    digitalWrite(ledPin, ledState);

    // Print LED state
    if (ledState) {
      Serial.println("ON");
    } else {
      Serial.println("OFF");
    }
  }
}
