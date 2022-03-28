/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
#define IS_ESP32
#define IS_SERIAL
//#define IS_LASER
#define IS_MOTOR
#define IS_DIGITAL
#define IS_DAC
//#define IS_WIFI
#define IS_PS3 // ESP32-only
//#define IS_ANALOG// ESP32-only
#define IS_LEDARR


// GALVos are always connected to 25/26 


// analog out (e.g. Lenses)
int analog_PIN_1 = 0;
int analog_PIN_2 = 0;
int analog_PIN_3 = 0;

// Stepper Motor pins
int STEP_A = 0;
int STEP_X = 18;
int STEP_Y = 19;
int STEP_Z = 23;
int DIR_A = 0; // on wemos mini 35 does not work
int DIR_X = 2; // on wemos mini 35 does not work
int DIR_Y = 33;
int DIR_Z = 4; // on wemos mini 34 does not work
int ENABLE = 5;

// ledarray
int LED_ARRAY_PIN = 32; 

// digital out (e.g. camera trigger)
int digital_PIN_1 = 22; 
int digital_PIN_2 = 21; 
int digital_PIN_3 = 17; 

// Laser PWM pins for CNC Shield
int LASER_PIN_1 = 0; // X-endstop
int LASER_PIN_2 = 0;// Y-endstop
int LASER_PIN_3 = 0;// Z-endstop

const char* identifier_setup = "pindef_lightsheet_esp_tomo_galvo";
