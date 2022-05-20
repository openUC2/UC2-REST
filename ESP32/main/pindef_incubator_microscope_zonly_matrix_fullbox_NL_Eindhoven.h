/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
#define IS_ESP32
#define IS_SERIAL
//#define IS_LASER
#define IS_MOTOR
//#define IS_DIGITAL
//#define IS_DAC
//#define IS_DAC_FAKE
//#define IS_WIFI
#define IS_PS4 // ESP32-only
//#define IS_ANALOG// ESP32-only
#define IS_LEDARR

// analog out (e.g. Lenses)
int analog_PIN_1 = 0;
int analog_PIN_2 = 0;
int analog_PIN_3 = 0;
// Stepper Motor pins
int STEP_A = 0;
int STEP_X = 0;
// we have a different motor here...
int STEP_X_1 = 32;
int STEP_X_2 = 27;
int STEP_X_3 = 21;
int STEP_X_4 = 25;
int STEP_Y = 0;
int STEP_Z = 0;
int DIR_A = 0;
int DIR_X = 0;
int DIR_Y = 0;
int DIR_Z = 0;
int ENABLE = 0;


// GALVos are always connected to 25/26 
int dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
int dac_fake_2 = 0; // Coolant

// ledarray
int LED_ARRAY_PIN = 2; // FEED HOLD 

// digital out (e.g. camera trigger)
int digital_PIN_1 = 0; //  Cycle Start/Resume
int digital_PIN_2 = 0; //not used/reserved
int digital_PIN_3 = 0; //not used/reserved

const char* identifier_setup = "pindef_incubator_microscope_Eindhoven";
