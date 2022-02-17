#ifdef IS_MOTOR

// Custom function accessible by the API
void motor_act_fct() {
  if(DEBUG) Serial.println("motor_act_fct");
  int mspeed = jsonDocument["speed"];
  long mposition1 = jsonDocument["pos1"];
  long mposition2 = jsonDocument["pos2"];
  long mposition3 = jsonDocument["pos3"];
  int isabs = jsonDocument["isabs"];
  int isblock = jsonDocument["isblock"];
  int isen = jsonDocument["isen"];
  int currentposition = 0;
  jsonDocument.clear();

  /*
    // apply default jsonDocument if nothing was sent
    if (strcmp(axis, "null")==0) axis = 1;
    if (strcmp(mspeed, "null")==0) mspeed = 0;
    if (strcmp(mposition, "null")==0) mposition = 0;
    if (strcmp(isabs, "null")==0) isabs = 1;
    if (strcmp(isblock, "null")==0) isblock = 1;
    if (strcmp(isen, "null")==0) isen = 0;
  */

  if (DEBUG) {
    Serial.print("speed "); Serial.println(mspeed);
    Serial.print("position1 "); Serial.println(mposition1);
    Serial.print("position2 "); Serial.println(mposition2);
    Serial.print("position3 "); Serial.println(mposition3);
    Serial.print("isabs "); Serial.println(isabs);
    Serial.print("isblock "); Serial.println(isblock);
    Serial.print("isen "); Serial.println(isen);
  }

  digitalWrite(ENABLE, LOW);
  stepper_X.begin(mspeed);
  stepper_Y.begin(mspeed);
  stepper_Z.begin(mspeed);

  if (isabs) {
    mposition1 = mposition1 - POSITION_MOTOR_X;
    mposition2 = mposition2 - POSITION_MOTOR_Y;
    mposition3 = mposition3 - POSITION_MOTOR_Z;
  }

  // weird error in controller?
  if (not(mposition1 == 0 and mposition2 == 0 and mposition3 == 0)) {
    controller.rotate(mposition1, mposition2, mposition3);
  }

  if (not isen) digitalWrite(ENABLE, HIGH);
  POSITION_MOTOR_X += mposition1;
  POSITION_MOTOR_Y += mposition2;
  POSITION_MOTOR_Z += mposition3;

  jsonDocument["POSX"] = POSITION_MOTOR_X;
  jsonDocument["POSY"] = POSITION_MOTOR_Y;
  jsonDocument["POSZ"] = POSITION_MOTOR_Z;

}

void motor_set_fct() {
  int axis = jsonDocument["axis"];

  if (DEBUG) {
    Serial.print("axis "); Serial.println(axis);
  }

  int currentposition = jsonDocument["currentposition"];
  int maxspeed = jsonDocument["maxspeed"];
  int acceleration = jsonDocument["acceleration"];
  int pinstep = jsonDocument["pinstep"];
  int pindir = jsonDocument["pindir"];
  int isen = jsonDocument["isen"];

  if (DEBUG) {
    Serial.print("currentposition "); Serial.println(currentposition);
    Serial.print("maxspeed "); Serial.println(maxspeed);
    Serial.print("acceleration "); Serial.println(acceleration);
    Serial.print("pinstep "); Serial.println(pinstep);
    Serial.print("pindir "); Serial.println(pindir);
    Serial.print("isen "); Serial.println(isen);
  }

  if (currentposition != 0) {
    if (DEBUG) Serial.print("currentposition "); Serial.println(currentposition);
    switch (axis) {
      case 1:
        POSITION_MOTOR_X = currentposition; //stepper_X.setCurrentPosition(currentposition);break;
        break;
      case 2:
        POSITION_MOTOR_Y = currentposition; //stepper_Y.setCurrentPosition(currentposition);break;
        break;
      case 3:
        POSITION_MOTOR_Z = currentposition; //stepper_Z.setCurrentPosition(currentposition);break;
        break;
        
    }
  }
  if (maxspeed != 0) {
    switch (axis) {
      case 1:
        stepper_X.begin(maxspeed); //stepper_X.setMaxSpeed(maxspeed);
        break;
      case 2:
        stepper_Y.begin(maxspeed); //stepper_Y.setMaxSpeed(maxspeed);
        break;
      case 3:
        stepper_Z.begin(maxspeed); //stepper_Z.setMaxSpeed(maxspeed);
        break;
    }
  }
  if (pindir != 0 and pinstep != 0) {
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

  //if (DEBUG) Serial.print("isen "); Serial.println(isen);
  if (isen != 0 and isen) {
    digitalWrite(ENABLE, 0);
  }
  else if (isen != NULL and not isen) {
    digitalWrite(ENABLE, 1);
  }

  jsonDocument.clear();
  jsonDocument["return"] = 1;
}



// Custom function accessible by the API
void motor_get_fct() {
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
    default:
      break;
  }

  jsonDocument.clear();
  jsonDocument["position"] = mposition;
  //jsonDocument["speed"] = mspeed;
  //jsonDocument["maxspeed"] = mmaxspeed;
  jsonDocument["pinstep"] = pinstep;
  jsonDocument["pindir"] = pindir;
}


/*


   wrapper for HTTP requests


*/


#ifdef IS_WIFI
void motor_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  motor_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void motor_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  motor_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void motor_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  motor_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
