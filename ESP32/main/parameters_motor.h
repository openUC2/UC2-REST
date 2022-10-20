#include <AccelStepper.h>
#include "esp_task_wdt.h"

// for stepper.h
#define MOTOR_STEPS 200
#define SLEEP 0
#define MS1 0
#define MS2 0
#define MS3 0
#define RPM 120

bool isaccel = false;
bool isforever = false;
bool motor_enable = false;

// global variables for the motor
long mspeed0 = 1000;
long mspeed1 = 1000;
long mspeed2 = 1000;
long mspeed3 = 1000;
long mposition0 = 0;
long mposition1 = 0;
long mposition2 = 0;
long mposition3 = 0;
boolean isstop = 0;

long POSITION_MOTOR_A = 0;
long POSITION_MOTOR_X = 0;
long POSITION_MOTOR_Y = 0;
long POSITION_MOTOR_Z = 0;

int MOTOR_ACCEL = 5000;
int MOTOR_DECEL = 5000;
  
int isabs = true;
int isen = false;
bool isactive = false;

// Homing parameters
int ishomeX=0;
int ishomeY=0;
int ishomeZ=0;
int ishomeA=0;

// direction
int SIGN_A = 1;
int SIGN_X = 1;
int SIGN_Y = 1;
int SIGN_Z = 1;
static const int FULLSTEPS_PER_REV_A = 200;
static const int FULLSTEPS_PER_REV_X = 200;
static const int FULLSTEPS_PER_REV_Y = 200;
static const int FULLSTEPS_PER_REV_Z = 200;

long MAX_VELOCITY_A = 20000;
long MAX_VELOCITY_X = 20000;
long MAX_VELOCITY_Y = 20000;
long MAX_VELOCITY_Z = 20000;
long MAX_ACCELERATION_A = 100000;
long MAX_ACCELERATION_X = 100000;
long MAX_ACCELERATION_Y = 100000;
long MAX_ACCELERATION_Z = 100000;
