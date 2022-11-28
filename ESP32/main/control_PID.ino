#ifdef IS_PID

// Custom function accessible by the API
void PID_act_fct() {

  // here you can do something
  if (DEBUG) Serial.println("PID_act_fct");


  if (jsonDocument.containsKey("PIDactive"))
    PID_active = (int)jsonDocument["PIDactive"];
  if (jsonDocument.containsKey("Kp"))
    PID_Kp = jsonDocument["Kp"];
  if (jsonDocument.containsKey("Ki"))
    PID_Ki = jsonDocument["Ki"];
  if (jsonDocument.containsKey("Kd"))
    PID_Kd = jsonDocument["Kd"];
  if (jsonDocument.containsKey("target"))
    PID_target = jsonDocument["target"];
  if (jsonDocument.containsKey("PID_updaterate"))
    PID_updaterate = (int)jsonDocument["PID_updaterate"];

  if (not PID_active) {
    // force shutdown the motor
    mspeed1 = 0;
    stepper_X.setSpeed(mspeed1);
    stepper_X.setMaxSpeed(mspeed1);
    stepper_X.runSpeed();
  }



  jsonDocument.clear();
  jsonDocument["Kp"] = PID_Kp;
  jsonDocument["Ki"] = PID_Ki;
  jsonDocument["Kd"] = PID_Kd;
  jsonDocument["PID_updaterate"] = PID_updaterate;
  jsonDocument["PID"] = PID_active;
  jsonDocument["target"] = PID_target;

}

void PID_background() {
  // hardcoded for now:
  int N_sensor_avg = 50;
  int sensorpin = ADC_pin_0;

  // get rid of noise?
  float sensorValueAvg = 0;
  for (int imeas = 0; imeas < N_sensor_avg; imeas++) {
    sensorValueAvg += analogRead(sensorpin);
  }

  sensorValueAvg = (float)sensorValueAvg / (float)N_sensor_avg;
  long motorValue = returnControlValue(PID_target, sensorValueAvg, PID_Kp, PID_Ki, PID_Kd);
  isforever = 1; // run motor at certain speed
  mspeed1 = motorValue;
  setEnableMotor(true);
  stepper_X.setSpeed(mspeed1);
  stepper_X.setMaxSpeed(mspeed1);

}


long returnControlValue(float controlTarget, float sensorValue, float Kp, float Ki, float Kd) {
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



void PID_set_fct() {
  if (DEBUG) Serial.println("PID_set_fct");
  int PIDID = (int)jsonDocument["PIDID"];
  int PIDPIN = (int)jsonDocument["PIDPIN"];
  if (jsonDocument.containsKey("N_sensor_avg"))
    N_sensor_avg = (int)jsonDocument["N_sensor_avg"];

  switch (PIDID) {
    case 0:
      ADC_pin_0 = PIDPIN;
      break;
    case 1:
      ADC_pin_1 = PIDPIN;
      break;
    case 2:
      ADC_pin_2 = PIDPIN;
      break;
  }


  jsonDocument.clear();
  jsonDocument["PIDPIN"] = PIDPIN;
  jsonDocument["PIDID"] = PIDID;
}



// Custom function accessible by the API
void PID_get_fct() {
  if (DEBUG) Serial.println("PID_get_fct");
  int PIDID = (int)jsonDocument["PIDID"];
  int PIDPIN = 0;
  switch (PIDID) {
    case 0:
      PIDPIN = ADC_pin_0;
      break;
    case 1:
      PIDPIN = ADC_pin_1;
      break;
    case 2:
      PIDPIN = ADC_pin_2;
      break;
  }

  jsonDocument.clear();
  jsonDocument["N_sensor_avg"] = N_sensor_avg;
  jsonDocument["PIDPIN"] = PIDPIN;
  jsonDocument["PIDID"] = PIDID;
}


void setup_PID() {
  if (DEBUG) Serial.println("Setting up sensors...");
}



/*
   wrapper for HTTP requests
*/
void PID_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  PID_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void PID_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  PID_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void PID_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  PID_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
