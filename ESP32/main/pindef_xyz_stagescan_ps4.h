/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */


//#define IS_LASER
#define IS_MOTOR
#define IS_DIGITAL
//#define IS_DAC
#define IS_DAC_FAKE
//#define IS_WIFI
#define IS_PS4 // ESP32-only
//#define IS_ANALOG// ESP32-only
#define IS_LEDARR
#define IS_READSENSOR
#define IS_PID

// for reading analog input values
int ADC_pin_0 = 34;
int ADC_pin_1 = 0;
int ADC_pin_2 = 0;
int N_sensor_avg = 100;

// analog out (e.g. Lenses)
int analog_PIN_1 = 0;
int analog_PIN_2 = 0;
int analog_PIN_3 = 0;

// Stepper Motor pins
int STEP_A = 0;
int STEP_X = 26;
int STEP_Y = 25;
int STEP_Z = 17;
int DIR_A = 0;
int DIR_X = 16;
int DIR_Y = 27;
int DIR_Z = 14;
int ENABLE = 12;


// GALVos are always connected to 25/26 
int dac_fake_1 = 2; // RESET-ABORT just toggles between 1 and 0
int dac_fake_2 = 34; // Coolant

// ledarray
int LED_ARRAY_PIN = 4; // FEED HOLD 

// digital out (e.g. camera trigger)
int digital_PIN_1 = 35; //  Cycle Start/Resume
int digital_PIN_2 = 35; //not used/reserved
int digital_PIN_3 = 39; //not used/reserved

const char* identifier_setup = "pindef_lightsheet_tomo_galvo_espwemos";
