#pragma once
#include <String.h>
struct PINDEF{
    // analog out (e.g. Lenses)
    int analog_PIN_1;
    int analog_PIN_2;
    int analog_PIN_3;

    // Stepper Motor pins
    int STEP_A;
    int STEP_X;
    int STEP_Y;
    int STEP_Z;
    int DIR_A;
    int DIR_X;
    int DIR_Y;
    int DIR_Z;
    int ENABLE;

    // Laser PWM pins
    int LASER_PIN_1;// Spin Dir
    int LASER_PIN_2;//  Spin En
    int LASER_PIN_3;//X_END_STOP;//

    // digital out (e.g. camera trigger)
    int digital_PIN_1;
    int digital_PIN_2;
    int digital_PIN_3;
    const char* identifier_setup;

    // ledarray
    int LED_ARRAY_PIN;
    int LED_ARRAY_NUM;

    // GALVos are always connected to 25/26 
    int dac_fake_1; // RESET-ABORT just toggles between 1 and 0
    int dac_fake_2; // Coolant

    int ADC_pin_0;
    int ADC_pin_1;
    int ADC_pin_2;

    
};
