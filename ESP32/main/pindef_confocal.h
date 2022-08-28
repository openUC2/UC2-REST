/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
 
#include "pindef_WEMOS_d1_r32.h"
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
 
#include "pindef_WEMOS_d1_r32.h"





#define IS_SCANNER
 // ESP32-only

// Laser PWM pins for CNC Shield
int LASER_PIN_1 = 0; // was SPINDLEPWMPIN; // Spin Dir
int LASER_PIN_2 = Y_LIMIT_PIN; // was SPINDLE_ENABLE_PIN;//  Spin En
int LASER_PIN_3 = 0;// 

// Stepper Motor pins
int STEP_A = 0;
int STEP_X = 0;
int STEP_Y = 0;
int STEP_Z = Z_STEP_PIN;
int DIR_A = 0;
int DIR_X = 0;
int DIR_Y = 0;
int DIR_Z = Z_DIRECTION_PIN;
int ENABLE = STEPPERS_ENABLE_PIN;

// GALVos are always connected to 25/26 
int dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
int dac_fake_2 = 0; // Coolant

// ledarray
int LED_ARRAY_PIN = X_LIMIT_PIN; // was FEED_HOLD_PIN; // FEED HOLD 

// digital out (e.g. camera trigger)
int digital_PIN_1 = 0; //  Cycle Start/Resume
int digital_PIN_2 = 0; //not used/reserved
int digital_PIN_3 = 0; //not used/reserved

const char* identifier_setup = "pindef_confocal";
