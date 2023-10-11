#include "freertos/queue.h"

#define PIN_A 33
#define PIN_B 32

volatile int encoder_pos = 0;
QueueHandle_t encoderQueue;

void IRAM_ATTR handleAChange() {
  if(digitalRead(PIN_A) == digitalRead(PIN_B)) {
    encoder_pos++;
  } else {
    encoder_pos--;
  }
  xQueueSendFromISR(encoderQueue, (void *) &encoder_pos, NULL);
}

void IRAM_ATTR handleBChange() {
  if(digitalRead(PIN_A) != digitalRead(PIN_B)) {
    encoder_pos++;
  } else {
    encoder_pos--;
  }
  xQueueSendFromISR(encoderQueue, (void *) &encoder_pos, NULL);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_A), handleAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B), handleBChange, CHANGE);

  encoderQueue = xQueueCreate(10, sizeof(int));
  xTaskCreate(processEncoderData, "ProcessEncoder", 2048, NULL, 2, NULL);
}

void loop() {
  // Your main code here
}

void processEncoderData(void * parameter) {
  int position;
  for(;;) {
    if(xQueueReceive(encoderQueue, &position, portMAX_DELAY)) {
      Serial.println(position);
      // Additional data processing can be performed here
    }
  }
}
