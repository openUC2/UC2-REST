
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */

// analog out (e.g. Lenses)
int analogout_PIN_1 = 21;
int analogout_PIN_2 = 22;
int analogout_PIN_3 = 0;

// Stepper
#ifdef IS_ARDUINO
// Motor pins
int STEP_X = 2;
int STEP_Y = 3;
int STEP_Z = 4;
int DIR_X = 5;
int DIR_Y = 6;
int DIR_Z = 7;
int ENABLE = 8;
#else
// Motor pins - multicolour fluorescence
int STEP_X = 0;
int STEP_Y = 23;
int STEP_Z = 2;
int DIR_X = 4;
int DIR_Y = 4;
int DIR_Z = 4;
int ENABLE = 0;
//Lightsheet?
/*int ENABLE = 26;
int STEP_X = 32;
int STEP_Y = 25;
int STEP_Z = 2;
int DIR_X = 23;
int DIR_Y = 23;
int DIR_Z = 23;
*/
#endif

// Laser PWM pins
#ifdef IS_ARDUINO
// for CNC Shield
int LASER_PIN_1 = 9; // X-endstop
int LASER_PIN_2 = 10;// Y-endstop
int LASER_PIN_3 = 11;// Z-endstop
#else
int LASER_PIN_1 = 18;
int LASER_PIN_2 = 19;
int LASER_PIN_3 = 21;
#endif
