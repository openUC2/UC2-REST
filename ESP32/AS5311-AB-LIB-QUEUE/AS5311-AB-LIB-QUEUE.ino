#include "AS5311AB.h"

#define PIN_A 32
#define PIN_B 33

AS5311AB encoder(PIN_A, PIN_B, false);

void setup() {
    Serial.begin(115200);
    encoder.begin();
}

void loop() {
    Serial.println(encoder.getPosition());
    delay(1000);
}
