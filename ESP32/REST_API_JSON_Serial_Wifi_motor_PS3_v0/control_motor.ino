#ifdef IS_MOTOR

// Custom function accessible by the API
void motor_act_fct() {
  if (DEBUG) Serial.println("motor_act_fct");

  // assign default values to thhe variables
  if (jsonDocument.containsKey("speed")) {
    mspeed = jsonDocument["speed"];
  }

  if (jsonDocument.containsKey("pos1")) {
    mposition1 = jsonDocument["pos1"];
  }
  else {
    mposition1 = 0;
  }

  if (jsonDocument.containsKey("pos2")) {
    mposition2 = jsonDocument["pos2"];
  }
  else {
    mposition2 = 0;
  }

  if (jsonDocument.containsKey("pos3")) {
    mposition3 = jsonDocument["pos3"];
  }
  else {
    mposition3 = 0;
  }

  if (jsonDocument.containsKey("isabs")) {
    isabs = jsonDocument["isabs"];
  }
  else {
    isabs = 0;
  }

  if (jsonDocument.containsKey("isblock")) {
    isblock = jsonDocument["isblock"];
  }
  else {
    isblock = 1;
  }

  if (jsonDocument.containsKey("stop")) {
    isstop = jsonDocument["stop"];
  }
  else {
    isstop = 0;
  }

  if (jsonDocument.containsKey("isen")) {
    isen = jsonDocument["isen"];
  }
  else {
    isen = 1;
  }
  jsonDocument.clear();

  if (DEBUG) {
    Serial.print("speed "); Serial.println(mspeed);
    Serial.print("position1 "); Serial.println(mposition1);
    Serial.print("position2 "); Serial.println(mposition2);
    Serial.print("position3 "); Serial.println(mposition3);
    Serial.print("isabs "); Serial.println(isabs);
    Serial.print("isblock "); Serial.println(isblock);
    Serial.print("isen "); Serial.println(isen);
    Serial.print("isstop "); Serial.println(isstop);
  }

  if (isstop) {
    stepper_X.stop();
    stepper_Y.stop();
    stepper_Z.stop();
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

  stepper_X.startRotate(SIGN_X * mposition1);
  stepper_Y.startRotate(SIGN_Y * mposition2);
  stepper_Z.startRotate(SIGN_Z * mposition3);

  // weird error in controller?
  if (isblock) {
    if (DEBUG) Serial.println("Start rotation");
    // WARNING! DON't use the controller! Not working well with arduinojson!!!
    //controller.rotate(SIGN_X * mposition1, SIGN_Y * mposition2, SIGN_Z * mposition3);

    while (true) {
      unsigned wait_time_x = stepper_X.nextAction();
      unsigned wait_time_y = stepper_Y.nextAction();
      unsigned wait_time_z = stepper_Z.nextAction();
      if (not(wait_time_x or wait_time_y or wait_time_z)) {
        if (DEBUG) Serial.println("Shutting down motor motion");
        break;
      }
    }

    if (DEBUG) Serial.println("Done with rotation");
    if (not isen) digitalWrite(ENABLE, HIGH);

  }
  else {
    if (DEBUG) Serial.println("Start rotation in background");
  }

  // TODO: not true for non-blocking!
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
  int sign = jsonDocument["sign"];
  int accel = jsonDocument["accel"];
  int deccel = jsonDocument["deccel"];


  if (DEBUG) {
    Serial.print("currentposition "); Serial.println(currentposition);
    Serial.print("maxspeed "); Serial.println(maxspeed);
    Serial.print("acceleration "); Serial.println(acceleration);
    Serial.print("pinstep "); Serial.println(pinstep);
    Serial.print("pindir "); Serial.println(pindir);
    Serial.print("isen "); Serial.println(isen);
    Serial.print("accel "); Serial.println(accel);
    Serial.print("deccel "); Serial.println(deccel);
  }


  if (acceleration != 0) {
    if (accel != 0) {
      MOTOR_ACCEL = accel;
    }
    if (deccel != 0) {
      MOTOR_DECEL = 5000;
    }
    if (accel > 0 and deccel > 0) {
      if (DEBUG) Serial.println("Setting Up Linear speed accelearation");
      stepper_X.setSpeedProfile(stepper_X.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
      stepper_Y.setSpeedProfile(stepper_Y.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
      stepper_Z.setSpeedProfile(stepper_Z.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
    }
    else {
      if (DEBUG) Serial.println("Setting Up constant speed accelearation");
      stepper_X.setSpeedProfile(stepper_X.CONSTANT_SPEED);
      stepper_Y.setSpeedProfile(stepper_Y.CONSTANT_SPEED);
      stepper_Z.setSpeedProfile(stepper_Z.CONSTANT_SPEED);
    }
  }



  if (sign != 0) {
    if (DEBUG) Serial.print("sign "); Serial.println(sign);
    switch (axis) {
      case 1:
        SIGN_X = sign;
        break;
      case 2:
        SIGN_Y = sign;
        break;
      case 3:
        SIGN_Z = sign;
        break;

    }
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
  else if (isen != 0 and not isen) {
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
  int sign = 0;

  switch (axis) {
    case 1:
      if (DEBUG) Serial.println("AXIS 1");
      //mmaxspeed = stepper_X.maxSpeed();
      //mspeed = stepper_X.speed();
      mposition = POSITION_MOTOR_X;//stepper_X.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
      sign = SIGN_X;
      break;
    case 2:
      if (DEBUG) Serial.println("AXIS 2");
      //mmaxspeed = stepper_Y.maxSpeed();
      //mspeed = stepper_Y.speed();
      mposition = POSITION_MOTOR_Y;//stepper_Y.currentPosition();
      pinstep = STEP_Y;
      pindir = DIR_Y;
      sign = SIGN_Y;
      break;
    case 3:
      if (DEBUG) Serial.println("AXIS 3");
      //mmaxspeed = stepper_Z.maxSpeed();
      //mspeed = stepper_Z.speed();
      mposition = POSITION_MOTOR_Z;//stepper_Z.currentPosition();
      pinstep = STEP_Z;
      pindir = DIR_Z;
      sign = SIGN_Z;
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
  jsonDocument["sign"] = sign;
}


void setup_motor() {

  /*
     Motor related settings
  */
  Serial.println("Setting Up Motors");
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);

  MOTOR_ACCEL = 5000;
  MOTOR_DECEL = 5000;
  Serial.println("Setting Up Motor X");
  stepper_X.begin(RPM);
  stepper_X.enable();
  stepper_X.setMicrostep(1);
  //stepper_X.setSpeedProfile(stepper_X.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
  stepper_X.setSpeedProfile(stepper_X.CONSTANT_SPEED);
  stepper_X.move(10);
  stepper_X.move(-10);

  Serial.println("Setting Up Motor Y");
  stepper_Y.begin(RPM);
  stepper_Y.enable();
  stepper_Y.setMicrostep(1);
  //stepper_Y.setSpeedProfile(stepper_Y.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
  stepper_Y.setSpeedProfile(stepper_Y.CONSTANT_SPEED);
  stepper_Y.move(10);
  stepper_Y.move(-10);

  Serial.println("Setting Up Motor Z");
  stepper_Z.begin(RPM);
  stepper_Z.enable();
  stepper_Z.setMicrostep(1);
  //stepper_Z.setSpeedProfile(stepper_Z.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL);
  stepper_Z.setSpeedProfile(stepper_Z.CONSTANT_SPEED);
  stepper_Z.move(10);
  stepper_Z.move(-10);
  digitalWrite(ENABLE, HIGH);

}


void drive_motor_background() {

  unsigned wait_time_x = stepper_X.nextAction();
  unsigned wait_time_y = stepper_Y.nextAction();
  unsigned wait_time_z = stepper_Z.nextAction();

  /* TODO: reimplement!
    if(not wait_time_x){
      POSITION_MOTOR_X += sgn(mposition1);
    }
    if(not wait_time_y){
      POSITION_MOTOR_Y += sgn(mposition2);
    }
    if(not wait_time_z){
      POSITION_MOTOR_Z+= sgn(mposition3);
    }
  */

  if (not(wait_time_x or wait_time_y or wait_time_z)) {
    if (not isen) {
      digitalWrite(ENABLE, HIGH);
      isblock = true;
    }
  }
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
