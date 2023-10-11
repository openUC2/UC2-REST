#include "AS5311AB.h"

AS5311AB encoder(33, 32);

void setup() {
    Serial.begin(115200);
    encoder.begin();
}

void loop() {
    Serial.println(encoder.getPosition());
    delay(1000);
}
