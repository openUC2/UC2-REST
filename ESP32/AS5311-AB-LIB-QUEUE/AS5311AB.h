#ifndef AS5311_H
#define AS5311_H

#include <Arduino.h>

class AS5311AB
{

public:
  AS5311AB(uint8_t pinA, uint8_t pinB, bool is_isr_service_installed);
  void begin();
  int getPosition();

private:
  static uint8_t _pinA, _pinB;

  static void IRAM_ATTR handleAChange(void *arg);
  static void IRAM_ATTR handleBChange(void *arg);
  static void processEncoderData(void *parameter);
  static bool _is_isr_service_installed;

  static volatile int encoder_pos;
  static QueueHandle_t encoderQueue;
};
#endif
