// IR Decoder Example using Interrupts on ESP32

#include <Arduino.h>

// Variables
volatile int i;
volatile int sign;
volatile long value;
volatile float result;
const int clockpin = 33;
const int datapin = 32;
volatile unsigned long tempmicros;
volatile bool decoding = false;

// Function Declarations
void IRAM_ATTR handleClockInterrupt();
void IRAM_ATTR handleDataInterrupt();
void decode();

void setup() {
  Serial.begin(115200);
  pinMode(clockpin, INPUT_PULLUP); // Ensure pull-up for clockpin as the interrupt will trigger on falling edge
  pinMode(datapin, INPUT_PULLUP);  // Ensure pull-up for datapin as the interrupt will trigger on falling edge

  attachInterrupt(digitalPinToInterrupt(clockpin), handleClockInterrupt, FALLING); // Attach clockpin interrupt
  attachInterrupt(digitalPinToInterrupt(datapin), handleDataInterrupt, FALLING);   // Attach datapin interrupt
}

void loop() {
  // Do nothing here, the decoding is handled by interrupts
}

void IRAM_ATTR handleClockInterrupt() {
  if (decoding == false) {
    tempmicros = micros();
  } else {
    if ((micros() - tempmicros) > 500) {
      decoding = false;
      decode();
    }
  }
  decoding = !decoding;
}

void IRAM_ATTR handleDataInterrupt() {
  if (decoding) {
    if (i < 20) {
      value |= 1 << i;
    }
    if (i == 20) {
      sign = -1;
    }
    i++;
  }
}

void decode() {
  value = 0;
  sign = 1;
  i = 0;
  result = (value * sign) / 100.00;
  Serial.println(result, 2);
  delay(50);
}
