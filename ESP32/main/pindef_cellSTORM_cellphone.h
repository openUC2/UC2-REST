
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
#define IS_SERIAL
//#define IS_WIFI
#define IS_PS3 // ESP32-only
#define IS_ANALOG// ESP32-only
#define IS_LASER
#define IS_MOTOR
//#define IS_DAC


String identifier_setup = "cellSTORM_cellphone"; 

// analog out (e.g. Lenses)
int analog_PIN_1 = 21;
int analog_PIN_2 = 22;
int analog_PIN_3 = 0;

// Definition cellSTORM
int STEP_X = 32;
int STEP_Y = 25;
int STEP_Z = 2;
int DIR_X = 23;
int DIR_Y = 23;
int DIR_Z = 23;
int ENABLE = 19;


// Laser PWM pins
int LASER_PIN_1 = 27;
int LASER_PIN_2 = 0;
int LASER_PIN_3 = 0;


#define PIN_ENABLE 26
