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
int STEP_X = 26;
int STEP_Y = 25;
int STEP_Z = 17;
int DIR_X = 16;
int DIR_Y = 27;
int DIR_Z = 14;
int ENABLE = 12;

// ledarray
int LED_ARRAY_PIN = 32; 

// digital out (e.g. camera trigger)
int digital_PIN_1 = 36; 
int digital_PIN_2 = 39; 
int digital_PIN_3 = 34; 

const char* identifier_setup = "pindef_lightsheet_tomo_galvo_espwemos";
