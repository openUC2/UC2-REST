#include "pindef_WEMOS_d1_r32.h"

int STEP_PIN_X = WEMOS_D1_R32_X_STEP_PIN;
int STEP_PIN_Y = WEMOS_D1_R32_Y_STEP_PIN;
int STEP_PIN_Z = WEMOS_D1_R32_Z_STEP_PIN;
int STEP_PIN_A = 0;

int DIR_PIN_X = WEMOS_D1_R32_X_DIRECTION_PIN;
int DIR_PIN_Y = WEMOS_D1_R32_Y_DIRECTION_PIN;
int DIR_PIN_Z = WEMOS_D1_R32_Z_DIRECTION_PIN;
int DIR_PIN_A = 0;

int ENABLE_PIN = WEMOS_D1_R32_STEPPERS_ENABLE_PIN;

int LED_ARRAY_PIN = WEMOS_D1_R32_FEED_HOLD_PIN;
int LED_ARRAY_NUM = 64;

int DIGITAL_PIN_1 = 0;
int DIGITAL_PIN_2 = 0;
int DIGITAL_PIN_3 = 0;

int ANALOG_PIN_1 = 0;
int ANALOG_PIN_2 = 0;
int ANALOG_PIN_3 = 0;

int LASER_PIN_1 = WEMOS_D1_R32_SPINDLEPWMPIN;
int LASER_PIN_2 = WEMOS_D1_R32_SPINDLE_ENABLE_PIN;
int LASER_PIN_3 = 0;

int DAC_FAKE_PIN_1 = 0;
int DAC_FAKE_PIN_2 = 0;

const char* PS3Mac = "01:02:03:04:05:06";
const char* PS4Mac = "1a:2b:3c:01:01:01";

const char*  WifiSSID = "BenMur";//"UC2 - F8Team"; //"IPHT - Konf"; // "Blynk";
const char*  WifiPW = "MurBen3128"; //"_lachmannUC2"; //"WIa2!DcJ"; //"12345678";
const char*  WifiSSIDAP = "UC2";
String hostname = "youseetoo";
