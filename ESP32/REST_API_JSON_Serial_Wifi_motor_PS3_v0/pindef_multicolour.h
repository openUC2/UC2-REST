
/*
 * Important: Don't use any pins that are not "allowed" on the ESP (also not twice assigment!!)
 */

// analog out (e.g. Lenses)

int analogout_PIN_1 = 22;
int analogout_PIN_2 = 24;
int analogout_PIN_3 = 0;

// Motor pins - multicolour fluorescence
int STEP_X = 0;
int STEP_Y = 23;
int STEP_Z = 2;
int DIR_X = 4;
int DIR_Y = 4;
int DIR_Z = 4;
int ENABLE = 5;

int LASER_PIN_1 = 18;
int LASER_PIN_2 = 19;
int LASER_PIN_3 = 21;

String identifier_setup = "multicolour";
