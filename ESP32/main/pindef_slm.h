
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */

#define IS_SERIAL
//#define IS_WIFI
#define IS_SLM
#define IS_LEDARR

int TFT_RST = 4;
int TFT_DC = 2; //A0
int TFT_CS = 15; //CS
int TFT_MOSI = 23; //SDA
int TFT_CLK = 18; //SCK



int LED_ARRAY_PIN = 22; // FEED HOLD 

String identifier_setup = "pindef_SLM";
