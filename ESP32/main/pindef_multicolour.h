
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
// load modules


//#define IS_WIFI
#define IS_PS3 // ESP32-only
#define IS_ANALOG// ESP32-only
#define IS_LASER
#define IS_MOTOR
//#define IS_DAC
#define IS_LEDARR

// analog out (e.g. Lenses)
int analog_PIN_1 = 32;
int analog_PIN_2 = 24;
int analog_PIN_3 = 0;

// Motor pins - multicolour fluorescence
int STEP_X = 0;
int STEP_Y = 23;
int STEP_Z = 2;
int DIR_X = 0;
int DIR_Y = 22;
int DIR_Z = 4;
int ENABLE = 5;

int LASER_PIN_1 = 18;
int LASER_PIN_2 = 19;
int LASER_PIN_3 = 21;

String identifier_setup = "multicolour";
