
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */

#define IS_SERIAL
//#define IS_WIFI
//#define IS_PS3 // ESP32-only
//#define IS_ANALOG// ESP32-only
#define IS_LASER
#define IS_MOTOR
#define IS_DIGITAL
//#define IS_DAC

const char *mSSID = "Blynk"; //"BenMur"; //
const char *mPWD = "12345678"; // "MurBen3128";//

//const char *mSSID = "ATTNVezUEM";
//const char *mPWD = "t8bfmze5a#id";


String identifier_setup = "cellSTORM"; 

// analog out (e.g. Lenses)
int analog_PIN_1 = 25;
int analog_PIN_2 = 26;
int analog_PIN_3 = 0;

// Definition cellSTORM
int STEP_X = 21;
int STEP_Y = 22;
int STEP_Z = 23;
int DIR_X = 18;
int DIR_Y = 18;
int DIR_Z = 18;
int ENABLE = 19;

// Laser PWM pins
int LASER_PIN_1 = 27;
int LASER_PIN_2 = 0;
int LASER_PIN_3 = 0;

// digital out (e.g. camera trigger)
int digital_PIN_1 = 12; 
int digital_PIN_2 = 13; 
int digital_PIN_3 = 0; 
