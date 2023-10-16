#include "AS5311AB.h"

volatile int AS5311AB::_encoderPos = 0;
QueueHandle_t AS5311AB::_encoderQueue = nullptr;
int AS5311AB::_pinA = 0;
int AS5311AB::_pinB = 0;

AS5311AB::AS5311AB(int pinA, int pinB)
    {
        AS5311AB::_pinA = pinA;
        AS5311AB::_pinB = pinB;
    }

void AS5311AB::begin() {
    pinMode(_pinA, INPUT_PULLUP);
    pinMode(_pinB, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(_pinA), _handleAChange, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_pinB), _handleBChange, CHANGE);

    _encoderQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(_processEncoderData, "ProcessEncoder", 2048, NULL, 2, NULL);
}

int AS5311AB::getPosition() {
    int pos;
    xQueueReceive(_encoderQueue, &pos, 0); // Non-blocking read
    return pos;
}

void IRAM_ATTR AS5311AB::_handleAChange()
{
    if (digitalRead(digitalPinToInterrupt(AS5311AB::_pinA)) == digitalRead(digitalPinToInterrupt(AS5311AB::_pinB))) {
        AS5311AB::_encoderPos++;
    } else {
        AS5311AB::_encoderPos--;
    }
}

void IRAM_ATTR AS5311AB::_handleBChange()
{
    if (digitalRead(digitalPinToInterrupt(AS5311AB::_pinB)) == digitalRead(digitalPinToInterrupt(AS5311AB::_pinA))) {
        AS5311AB::_encoderPos--;
    } else {
        AS5311AB::_encoderPos++;
    }
}

void AS5311AB::_processEncoderData(void* parameter) {
    int position;
    for (;;) {
        if (xQueueReceive(_encoderQueue, &position, portMAX_DELAY)) {
            // Handle your data here, ensure you are not in an ISR when using certain functions (like Serial).
            Serial.println(position);
        }
    }
}
