/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
 
#include "pindef_WEMOS_d1_r32.h"
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
 
#include "pindef_WEMOS_d1_r32.h"

#define IS_ESP32
#define IS_SERIAL
#define IS_LASER
#define IS_MOTOR
//#define IS_DIGITAL
#define IS_DAC
//#define IS_DAC_FAKE
//#define IS_WIFI
#define IS_PS4 // ESP32-only
//#define IS_ANALOG// ESP32-only
#define IS_LEDARR

// analog out (e.g. Lenses)
int analog_PIN_1 = 0;
int analog_PIN_2 = 0;
int analog_PIN_3 = 0;

// Laser PWM pins for CNC Shield
int LASER_PIN_1 = 34; // was SPINDLEPWMPIN; // Spin Dir
int LASER_PIN_2 = 32; // was SPINDLE_ENABLE_PIN;//  Spin En
int LASER_PIN_3 = 0;// 

int LIM_X = 17;
int LIM_Y = 4;
int LIM_Z = 15;

// Stepper Motor pins
int STEP_A = 22;
int STEP_X = 2;
int STEP_Y = 27;
int STEP_Z = 12;
int DIR_A = 21;
int DIR_X = 31;
int DIR_Y = 16;
int DIR_Z = 14;
int ENABLE = 13;

// GALVos are always connected to 25/26 
int dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
int dac_fake_2 = 0; // Coolant

// ledarray
int LED_ARRAY_PIN = 35; //CYCLE_START_PIN; // was FEED_HOLD_PIN; // FEED HOLD 

// digital out (e.g. camera trigger)
int digital_PIN_1 = 0; //  Cycle Start/Resume
int digital_PIN_2 = 0; //not used/reserved
int digital_PIN_3 = 0; //not used/reserved

const char* identifier_setup = "pindef_uc2standalone";
