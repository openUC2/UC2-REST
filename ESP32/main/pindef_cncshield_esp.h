
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
#define IS_LEDARR
//#define IS_DAC

const char *mSSID = "Blynk"; //"BenMur"; //
const char *mPWD = "12345678"; // "MurBen3128";//

//const char *mSSID = "ATTNVezUEM";
//const char *mPWD = "t8bfmze5a#id";


String identifier_setup = "ESP32_CNC_SHIELD"; 

/*
Ard - ESP - CNC
0 -RX
1 TX
2 - 26 - xstep
3 - 25 - ystep 
4 17 - zstepp
5 16 - xdir
6 27 - ydir
7 14 - zdir
8 12 - EN
9 13 - X-Endstop
10 5 - Y-Endstop
11 23 - Z-Endstop
12 19 - SpinEnable
13 18 - SpinDir

A0 - 2
A1 - 4
A2 - 35
A3 - 34. Coolant
A4 - 36 -  A4 - 
A5 - 39 -  A5 - 
*/


// analog out (e.g. Lenses)
int analog_PIN_1 = 0;
int analog_PIN_2 = 0;
int analog_PIN_3 = 0;

// Definition cellSTORM
int STEP_X = 26;
int STEP_Y = 25;
int STEP_Z = 17;
int DIR_X = 16;
int DIR_Y = 27;
int DIR_Z = 14;
int ENABLE = 12;

// Laser PWM pins
int LASER_PIN_1 = 13;
int LASER_PIN_2 = 5;
int LASER_PIN_3 = 23;

// digital out (e.g. camera trigger)
int digital_PIN_1 = 36; 
int digital_PIN_2 = 39; 
int digital_PIN_3 = 34; 
