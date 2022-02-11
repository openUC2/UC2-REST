
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */
String identifier_setup = "cellSTORM"; 

// analog out (e.g. Lenses)
int analogout_PIN_1 = 25;
int analogout_PIN_2 = 26;
int analogout_PIN_3 = 0;

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


#define PIN_ENABLE 19
