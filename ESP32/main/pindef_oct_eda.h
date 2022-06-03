/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
 
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
 
#include "pindef_WEMOS_d1_r32.h"

#define IS_ESP32
#define IS_SERIAL
#define IS_LASER

bool IS_PSCONTROLER_ACTIVE = 0;

// Laser PWM pins for CNC Shield
int LASER_PIN_1 = 27; // was SPINDLEPWMPIN; // Spin Dir
int LASER_PIN_2 = 0; // was SPINDLE_ENABLE_PIN;//  Spin En
int LASER_PIN_3 = 0;// 

const char* identifier_setup = "pindef_oct_eda";
