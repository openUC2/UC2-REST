#include "AS5311.h"

AS5311 sensor(GPIO_NUM_26, GPIO_NUM_27);

void setup() {
    //Serial.begin(115200);
    //sensor.begin();
}

void loop() {
    /*
    float position = sensor.readPosition();
    int edgeCount = sensor.readEdgeCounter();
    if (position != -1.0f) {
        Serial.println(position);
        Serial.println(edgeCount);
    }
    delay(10);
    */
}
