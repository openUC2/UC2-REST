// IR Decoder Example

// Libraries
#include <Arduino.h>

// Variables
int i;
int sign;
long value;
float result;
int clockpin = 33;
int datapin = 32;
unsigned long tempmicros;

// Function Declarations
void setup();
void loop();
void decode();

// Setup Function - Runs once at the start
void setup() {
  Serial.begin(115200);  // Initialize Serial communication at 115200 baud rate
  pinMode(clockpin, INPUT);  // Set clockpin as an input
  pinMode(datapin, INPUT);   // Set datapin as an input
}

// Loop Function - Runs repeatedly
void loop() {
  while (digitalRead(clockpin) == HIGH) {} // If clock is HIGH, wait until it turns to LOW
  tempmicros = micros(); // Record the current micros() value

  while (digitalRead(clockpin) == LOW) {} // Wait for the end of the HIGH pulse

  if ((micros() - tempmicros) > 500) { // If the HIGH pulse was longer than 500 microseconds, we are at the start of a new bit sequence
    decode(); // Decode the bit sequence
  }
}

// Decode Function - Decodes the received IR signal
void decode() {
  sign = 1; // Initialize sign to positive
  value = 0; // Initialize value to zero

  for (i = 0; i < 23; i++) {
    while (digitalRead(clockpin) == HIGH) {} // Wait until clock returns to HIGH (the first bit is not needed)
    while (digitalRead(clockpin) == LOW) {} // Wait until clock returns to LOW

    if (digitalRead(datapin) == LOW) {
      if (i < 20) {
        value |= 1 << i; // Set the bit in 'value' at position 'i' to 1
      }
      if (i == 20) {
        sign = -1; // Set sign to negative
      }
    }
  }

  result = (value * sign) / 100.00; // Calculate the result (value with sign and two decimal places)
  Serial.println(result); // Print the result with 2 decimal places
  delay(50); // Delay for a short time to avoid continuous decoding
}
