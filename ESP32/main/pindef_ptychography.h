
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
#define IS_SERIAL
#define IS_LASER
#define IS_MOTOR
//#define IS_DAC
//#define IS_WIFI
//#define IS_PS3 // ESP32-only
//#define IS_ANALOG// ESP32-only


// analog out (e.g. Lenses)
int analog_PIN_1 = 21;
int analog_PIN_2 = 22;
int analog_PIN_3 = 0;

// Stepper Motor pins
int STEP_X = 2;
int STEP_Y = 3;
int STEP_Z = 4;
int DIR_X = 5;
int DIR_Y = 6;
int DIR_Z = 7;
int ENABLE = 8;

// Laser PWM pins for CNC Shield
int LASER_PIN_1 = 9; // X-endstop
int LASER_PIN_2 = 10;// Y-endstop
int LASER_PIN_3 = 11;// Z-endstop

const char* identifier_setup = "pindef_ptychography";
