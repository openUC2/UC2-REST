#include "AS5311.h"

AS5311 sensor(GPIO_NUM_26, 27);

void setup() {
    Serial.begin(115200);
    sensor.begin();
}

void loop() {
    float position = sensor.readPosition();
    if (position != -1.0f) {
        Serial.println(position);
    }
    delay(10);
}
