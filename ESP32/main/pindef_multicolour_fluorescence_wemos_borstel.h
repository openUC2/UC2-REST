/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
 
#include "pindef_WEMOS_d1_r32.h"

#define IS_ESP32
#define IS_SERIAL
#define IS_LASER
#define IS_MOTOR
//#define IS_DIGITAL
//#define IS_DAC
//#define IS_DAC_FAKE
//#define IS_WIFI
#define IS_PS3 // ESP32-only
//#define IS_ANALOG// ESP32-only
#define IS_LEDARR

// analog out (e.g. Lenses)
int analog_PIN_1 = 0;
int analog_PIN_2 = 0;
int analog_PIN_3 = 0;

// Laser PWM pins for CNC Shield
int LASER_PIN_1 = SPINDLEPWMPIN; // Spin Dir
int LASER_PIN_2 = SPINDLE_ENABLE_PIN;//  Spin En
int LASER_PIN_3 = 0;// 


// Stepper Motor pins
int STEP_A = 0;
int STEP_X = X_STEP_PIN;
int STEP_Y = Y_STEP_PIN;
int STEP_Z = Z_STEP_PIN;
int DIR_A = 0;
int DIR_X = X_DIRECTION_PIN;
int DIR_Y = Y_DIRECTION_PIN;
int DIR_Z = Z_DIRECTION_PIN;
int ENABLE = STEPPERS_ENABLE_PIN;

// GALVos are always connected to 25/26 
int dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
int dac_fake_2 = 0; // Coolant

// ledarray
int LED_ARRAY_PIN = FEED_HOLD_PIN; // FEED HOLD 

// digital out (e.g. camera trigger)
//int digital_PIN_1 = 0; //  Cycle Start/Resume
//int digital_PIN_2 = 0; //not used/reserved
//int digital_PIN_3 = 0; //not used/reserved

const char* identifier_setup = "pindef_multicolour_fluorescence_wemos_borstel";
