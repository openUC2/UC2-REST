
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */

#define IS_DAC // ESP32-only

// analog out (e.g. Lenses)
int analogout_PIN_1 = 0;
int analogout_PIN_2 = 0;
int analogout_PIN_3 = 0;

// Definition cellSTORM
int STEP_X = 2;
int STEP_Y = 0;
int STEP_Z = 0;
int DIR_X = 23;
int DIR_Y = 23;
int DIR_Z = 23;
int ENABLE = 22;

// Laser PWM pins
int LASER_PIN_1 = 32;
int LASER_PIN_2 = 0;
int LASER_PIN_3 = 0;

String identifier_setup = "pindef_lightsheet";
