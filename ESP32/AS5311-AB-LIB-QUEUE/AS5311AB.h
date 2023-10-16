#ifndef AS5311AB_H
#define AS5311AB_H

#include "Arduino.h"
#include "freertos/queue.h"

class AS5311AB {
public:
    AS5311AB(int pinA, int pinB);
    void begin();
    int getPosition();

private:
    static int _pinA;
    static int _pinB;
    static volatile int _encoderPos;
    static QueueHandle_t _encoderQueue;


    static void IRAM_ATTR _handleAChange();
    static void IRAM_ATTR _handleBChange();
    static void _processEncoderData(void* parameter);
};

#endif
