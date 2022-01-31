#ifdef IS_MOTOR

// Custom function accessible by the API
DynamicJsonDocument motor_act_fct(JsonDocument& Values) {
  Serial.println("motor_act_fct");

  int axis = Values["axis"];
  int mspeed = Values["speed"];
  long mposition = Values["position"];
  int isabsolute = Values["isabsolute"];
  int isblocking = Values["isblocking"];

  /*
  // apply default values if nothing was sent
  if (axis == "null") axis = 1;
  if (mspeed == "null") axis = 0;
  if (mposition == "null") axis = 0;
  if (isabsolute == "null") axis = 1;
  if (isblocking == "null") axis = 1;
  */
  
  if (DEBUG) {
    Serial.print("axis "); Serial.println(axis);
    Serial.print("speed "); Serial.println(mspeed);
    Serial.print("position "); Serial.println(mposition);
    Serial.print("isabsolute "); Serial.println(isabsolute);
    Serial.print("isblocking "); Serial.println(isblocking);
  }

  /*
    if (mspeed == 0) {
    stepper_X.stop();
    stepper_Y.stop();
    stepper_Z.stop();
    }
  */

  if (axis == 1) {
    stepper_X.begin(mspeed);
    if (isabsolute) stepper_X.move(mposition * steps_per_mm_X);
    else stepper_X.move(mposition * steps_per_mm_X);
    POSITION_MOTOR_X = - mposition * steps_per_mm_X;
    //if(isblocking) stepper_X.runToPosition();
  }
  else if (axis == 2) {
    stepper_Y.begin(mspeed);
    if (isabsolute) stepper_Y.move(mposition * steps_per_mm_Y);
    else stepper_Y.move(mposition * steps_per_mm_Y);
    POSITION_MOTOR_Y = - mposition * steps_per_mm_Y;
    /*
      stepper_Y.setSpeed((float)mspeed * steps_per_mm_Y);
      if (isabsolute) stepper_Y.moveTo(mposition * steps_per_mm_Y);
      else stepper_Y.moveTo(mposition * steps_per_mm_Y);
      if(isblocking) stepper_Y.runToPosition();
    */
  }
  else if (axis == 3) {
    stepper_Z.begin(mspeed);
    if (isabsolute) stepper_Z.move(mposition * steps_per_mm_Z);
    else stepper_Z.move(mposition * steps_per_mm_Z);
    POSITION_MOTOR_Z = - mposition * steps_per_mm_Z;
  }

  Values.clear();
  Values["return"] = 1;

  return Values ;
}

DynamicJsonDocument motor_set_fct(JsonDocument& Values) {
  int axis = Values["axis"];

  if (DEBUG) {
    Serial.print("axis "); Serial.println(axis);
  }

  int currentposition = jsonDocument["currentposition"];
  int maxspeed = jsonDocument["maxspeed"];
  int acceleration = jsonDocument["acceleration"];
  int pinstep = jsonDocument["pinstep"];
  int pindir = jsonDocument["pindir"];
  int isenabled = Values["isenabled"];


  if (currentposition != NULL) {
    if (DEBUG) Serial.print("currentposition "); Serial.println(currentposition);
    switch (axis) {
      case 1:
        POSITION_MOTOR_X = currentposition; //stepper_X.setCurrentPosition(currentposition);break;
      case 2:
        POSITION_MOTOR_Y = currentposition; //stepper_Y.setCurrentPosition(currentposition);break;
      case 3:
        POSITION_MOTOR_Z = currentposition; //stepper_Z.setCurrentPosition(currentposition);break;
    }
  }
  if (maxspeed != NULL) {
    switch (axis) {
      case 1:
        stepper_X.begin(maxspeed); //stepper_X.setMaxSpeed(maxspeed);break;
      case 2:
        stepper_Y.begin(maxspeed); //stepper_Y.setMaxSpeed(maxspeed);break;
      case 3:
        stepper_Z.begin(maxspeed); //stepper_Z.setMaxSpeed(maxspeed);break;
    }
  }

  /*
    if (acceleration != NULL) {
      switch (axis) {
        case 1:
          stepper_X.setAcceleration(acceleration);break;
        case 2:
          stepper_Y.setAcceleration(acceleration);break;
        case 3:
          stepper_Z.setAcceleration(acceleration);break;
      }
    }
  */

  if (pindir != NULL and pinstep != NULL) {
    if (axis == 1) {
      STEP_X = pinstep;
      DIR_X = pindir;
      A4988 stepper_X(FULLSTEPS_PER_REV_X, DIR_X, STEP_X, SLEEP, MS1, MS2, MS3); //stepper_X = AccelStepper(AccelStepper::DRIVER, STEP_X, DIR_X);
    }
    else if (axis == 2) {
      STEP_Y = pinstep;
      DIR_Y = pindir;
      A4988 stepper_X(FULLSTEPS_PER_REV_Y, DIR_Y, STEP_Y, SLEEP, MS1, MS2, MS3); //stepper_Y = AccelStepper(AccelStepper::DRIVER, STEP_Y, DIR_Y);
    }
    else if (axis == 3) {
      STEP_Z = pinstep;
      DIR_Z = pindir;
      A4988 stepper_X(FULLSTEPS_PER_REV_Z, DIR_Z, STEP_Z, SLEEP, MS1, MS2, MS3); //stepper_Z = AccelStepper(AccelStepper::DRIVER, STEP_Z, DIR_Z);
    }
  }

  if (DEBUG) Serial.print("isenabled "); Serial.println(isenabled);
  if (isenabled != NULL and isenabled) {
    digitalWrite(ENABLE, 0);
  }
  else if (isenabled != NULL and not isenabled) {
    digitalWrite(ENABLE, 1);
  }

  Values.clear();
  Values["return"] = 1;

  return Values ;
}





// Custom function accessible by the API
DynamicJsonDocument motor_get_fct(JsonDocument& Values) {
  int axis = jsonDocument["axis"];
  if (DEBUG) Serial.println("motor_get_fct");
  if (DEBUG) Serial.println(axis);

  //int mmaxspeed = 0;
  //int mspeed = 0;
  int mposition = 0;
  int pinstep = 0;
  int pindir = 0;

  switch (axis) {
    case 1:
      if (DEBUG) Serial.println("AXIS 1");
      //mmaxspeed = stepper_X.maxSpeed();
      //mspeed = stepper_X.speed();
      mposition = POSITION_MOTOR_X;//stepper_X.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
      break;
    case 2:
      if (DEBUG) Serial.println("AXIS 2");
      //mmaxspeed = stepper_Y.maxSpeed();
      //mspeed = stepper_Y.speed();
      mposition = POSITION_MOTOR_Y;//stepper_Y.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
      break;
    case 3:
      if (DEBUG) Serial.println("AXIS 3");
      //mmaxspeed = stepper_Z.maxSpeed();
      //mspeed = stepper_Z.speed();
      mposition = POSITION_MOTOR_Z;//stepper_Z.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
      break;
  }

  jsonDocument.clear();
  jsonDocument["position"] = mposition;
  //jsonDocument["speed"] = mspeed;
  //jsonDocument["maxspeed"] = mmaxspeed;
  jsonDocument["pinstep"] = pinstep;
  jsonDocument["pindir"] = pindir;
  return jsonDocument;
}


/*


   wrapper for HTTP requests


*/


#ifdef IS_WIFI
void motor_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = motor_act_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void motor_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = motor_get_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void motor_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = motor_set_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
