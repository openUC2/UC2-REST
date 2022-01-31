#ifdef IS_MOTOR

// Custom function accessible by the API
DynamicJsonDocument motor_act_fct(JsonDocument& Values) {
  Serial.println("motor_act_fct");

  int axis = Values["axis"];
  int mspeed = Values["speed"];
  long mposition = Values["position"];
  int isabsolute = Values["isabsolute"];
  int isblocking = Values["isblocking"];

  // apply default values if nothing was sent
  if(axis == NULL) axis = 1;
  if(mspeed == NULL) axis = 0;
  if(mposition == NULL) axis = 0;
  if(isabsolute == NULL) axis = 1;
  if(isblocking == NULL) axis = 1;
  
  if (DEBUG) {
    Serial.print("axis "); Serial.println(axis);
    Serial.print("speed "); Serial.println(mspeed);
    Serial.print("position "); Serial.println(mposition);
    Serial.print("isabsolute "); Serial.println(isabsolute);
    Serial.print("isblocking "); Serial.println(isblocking);
  }

  if (mspeed == 0) {
    stepper_X.stop();
    stepper_Y.stop();
    stepper_Z.stop();
  }

  switch (axis) {
    case 1:
      stepper_X.setSpeed((float)mspeed * steps_per_mm_X);
      if (isabsolute) stepper_X.moveTo(mposition * steps_per_mm_X);
      else stepper_X.move(mposition * steps_per_mm_X);
      if(isblocking) stepper_X.runToPosition();
      break;
    case 2:
      stepper_Y.setSpeed((float)mspeed * steps_per_mm_Y);
      if (isabsolute) stepper_Y.moveTo(mposition * steps_per_mm_Y);
      else stepper_Y.moveTo(mposition * steps_per_mm_Y);
      if(isblocking) stepper_Y.runToPosition();
      break;
    case 3:
      stepper_Z.setSpeed((float)mspeed * steps_per_mm_Z);
      if (isabsolute) stepper_Z.moveTo(mposition * steps_per_mm_Z);
      else stepper_Z.move(mposition * steps_per_mm_Z);
      if(isblocking) stepper_Z.runToPosition();
    default:
      // Statement(s)
      break;
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
        stepper_X.setCurrentPosition(currentposition);break;
      case 2:
        stepper_Y.setCurrentPosition(currentposition);break;
      case 3:
        stepper_Z.setCurrentPosition(currentposition);break;
    }
  }
  if (maxspeed != NULL) {
    switch (axis) {
      case 1:
        stepper_X.setMaxSpeed(maxspeed);break;
      case 2:
        stepper_Y.setMaxSpeed(maxspeed);break;
      case 3:
        stepper_Z.setMaxSpeed(maxspeed);break;
    }
  }

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

  if (pindir != NULL and pinstep != NULL) {
    switch (axis) {
      case 1:
        STEP_X = pinstep;
        DIR_X = pindir;
        stepper_X = AccelStepper(AccelStepper::DRIVER, STEP_X, DIR_X);
        break;
      case 2:
        STEP_Y = pinstep;
        DIR_Y = pindir;
        stepper_Y = AccelStepper(AccelStepper::DRIVER, STEP_Y, DIR_Y);
        break;
      case 3:
        STEP_Z = pinstep;
        DIR_Z = pindir;
        stepper_Z = AccelStepper(AccelStepper::DRIVER, STEP_Z, DIR_Z);
        break;
    }
  }

  if (DEBUG) Serial.print("isenabled "); Serial.println(isenabled);
  if (isenabled != NULL and isenabled){
    digitalWrite(ENABLE, 0);}
  else if (isenabled != NULL and not isenabled){
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

  int mmaxspeed = 0;
  int mspeed = 0;
  int mposition = 0;
  int pinstep = 0;
  int pindir = 0;

  switch (axis) {
    case 1:
      if(DEBUG) Serial.println("AXIS 1");
      mmaxspeed = stepper_X.maxSpeed();
      mspeed = stepper_X.speed();
      mposition = stepper_X.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
      break;
    case 2:
    if(DEBUG) Serial.println("AXIS 2");
      mmaxspeed = stepper_Y.maxSpeed();
      mspeed = stepper_Y.speed();
      mposition = stepper_Y.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
      break;
    case 3:
    if(DEBUG) Serial.println("AXIS 3");
      mmaxspeed = stepper_Z.maxSpeed();
      mspeed = stepper_Z.speed();
      mposition = stepper_Z.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
      break;
  }

  jsonDocument.clear();
  jsonDocument["position"] = mposition;
  jsonDocument["speed"] = mspeed;
  jsonDocument["maxspeed"] = mmaxspeed;
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
