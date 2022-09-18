#include "../../config.h"
#include "PidController.h"


PidController::PidController(/* args */){};
PidController::~PidController(){};

// Custom function accessible by the API
void PidController::act() {

  // here you can do something
  if (DEBUG) Serial.println("PID_act_fct");


  if (WifiController::getJDoc()->containsKey("PIDactive"))
    PID_active = (int)(*WifiController::getJDoc())["PIDactive"];
  if (WifiController::getJDoc()->containsKey("Kp"))
    PID_Kp = (*WifiController::getJDoc())["Kp"];
  if ((*WifiController::getJDoc()).containsKey("Ki"))
    PID_Ki = (*WifiController::getJDoc())["Ki"];
  if (WifiController::getJDoc()->containsKey("Kd"))
    PID_Kd = (*WifiController::getJDoc())["Kd"];
  if (WifiController::getJDoc()->containsKey("target"))
    PID_target = (*WifiController::getJDoc())["target"];
  if (WifiController::getJDoc()->containsKey("PID_updaterate"))
    PID_updaterate = (int)(*WifiController::getJDoc())["PID_updaterate"];

  if (!PID_active) {
    // force shutdown the motor
    #ifdef IS_MOTOR
    motor.mspeed1 = 0;
    motor.stepper_X->setSpeed(motor.mspeed1);
    motor.stepper_X->setMaxSpeed(motor.mspeed1);
    motor.stepper_X->runSpeed();
    #endif
  }



  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["Kp"] = PID_Kp;
  (*WifiController::getJDoc())["Ki"] = PID_Ki;
  (*WifiController::getJDoc())["Kd"] = PID_Kd;
  (*WifiController::getJDoc())["PID_updaterate"] = PID_updaterate;
  (*WifiController::getJDoc())["PID"] = PID_active;
  (*WifiController::getJDoc())["target"] = PID_target;

}

void PidController::background() {
  // hardcoded for now:
  int N_sensor_avg = 50;
  int sensorpin = pins->ADC_pin_0;

  // get rid of noise?
  float sensorValueAvg = 0;
  for (int imeas = 0; imeas < N_sensor_avg; imeas++) {
    sensorValueAvg += analogRead(sensorpin);
  }

  sensorValueAvg = (float)sensorValueAvg / (float)N_sensor_avg;
  long motorValue = returnControlValue(PID_target, sensorValueAvg, PID_Kp, PID_Ki, PID_Kd);
  #ifdef IS_MOTOR
  motor.isforever = 1; // run motor at certain speed
  motor.mspeed1 = motorValue;
  motor.setEnableMotor(true);
  motor.stepper_X->setSpeed(motor.mspeed1);
  motor.stepper_X->setMaxSpeed(motor.mspeed1);
  #endif
}


long PidController::returnControlValue(float controlTarget, float sensorValue, float Kp, float Ki, float Kd) {
  float sensorOffset = 0.;
  float maxError = 1.;
  float error = (controlTarget - (sensorValue - sensorOffset)) / maxError;
  float cP = Kp * error;
  float cI = Ki * errorRunSum;
  float cD = Kd * (error - previousError);
  float PID = cP + cI + cD;
  long stepperOut = (long)PID;

  if (stepperOut > stepperMaxValue) {
    stepperOut = stepperMaxValue;
  }

  if (stepperOut < -stepperMaxValue) {
    stepperOut = -stepperMaxValue;
  }


  errorRunSum = errorRunSum + error;
  previousError = error;

  if (DEBUG) Serial.println("sensorValue: " + String(sensorValue) + ", P: " + String(cP) + ", I: " + String(cI) + ", D: " + String(cD) + ", errorRunSum: " + String(errorRunSum) + ", previousError: " + String(previousError) + ", stepperOut: " + String(stepperOut));
  return stepperOut;
}



void PidController::set() {
  if (DEBUG) Serial.println("PID_set_fct");
  int PIDID = (int)(*WifiController::getJDoc())["PIDID"];
  int PIDPIN = (int)(*WifiController::getJDoc())["PIDPIN"];
  if (WifiController::getJDoc()->containsKey("N_sensor_avg"))
    N_sensor_avg = (int)(*WifiController::getJDoc())["N_sensor_avg"];

  switch (PIDID) {
    case 0:
      pins->ADC_pin_0 = PIDPIN;
      break;
    case 1:
      pins->ADC_pin_1 = PIDPIN;
      break;
    case 2:
      pins->ADC_pin_2 = PIDPIN;
      break;
  }


  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["PIDPIN"] = PIDPIN;
  (*WifiController::getJDoc())["PIDID"] = PIDID;
}



// Custom function accessible by the API
void PidController::get() {
  if (DEBUG) Serial.println("PID_get_fct");
  int PIDID = (int)(*WifiController::getJDoc())["PIDID"];
  int PIDPIN = 0;
  switch (PIDID) {
    case 0:
      PIDPIN = pins->ADC_pin_0;
      break;
    case 1:
      PIDPIN = pins->ADC_pin_1;
      break;
    case 2:
      PIDPIN = pins->ADC_pin_2;
      break;
  }

  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["N_sensor_avg"] = N_sensor_avg;
  (*WifiController::getJDoc())["PIDPIN"] = PIDPIN;
  (*WifiController::getJDoc())["PIDID"] = PIDID;
}


void PidController::setup(PINDEF * pins) {
  this->pins = pins;
  if (DEBUG) Serial.println("Setting up sensors...");
}

