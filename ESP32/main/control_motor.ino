#ifdef IS_MOTOR



// Custom function accessible by the API
void motor_act_fct() {
  if (DEBUG) Serial.println("motor_act_fct");


  // assign default values to thhe variables
  if (jsonDocument.containsKey("speed0")) {
    mspeed0 = jsonDocument["speed0"];
  }
  else if (jsonDocument.containsKey("speed")) {
    mspeed0 = jsonDocument["speed"];
  }
  if (jsonDocument.containsKey("speed1")) {
    mspeed1 = jsonDocument["speed1"];
  }
  else if (jsonDocument.containsKey("speed")) {
    mspeed1 = jsonDocument["speed"];
  }
  if (jsonDocument.containsKey("speed2")) {
    mspeed2 = jsonDocument["speed2"];
  }
  else if (jsonDocument.containsKey("speed")) {
    mspeed2 = jsonDocument["speed"];
  }
  if (jsonDocument.containsKey("speed3")) {
    mspeed3 = jsonDocument["speed3"];
  }
  else if (jsonDocument.containsKey("speed")) {
    mspeed3 = jsonDocument["speed"];
  }

  if (jsonDocument.containsKey("pos0")) {
    mposition0 = jsonDocument["pos0"];
  }
  else {
    mposition0 = 0;
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

  if (jsonDocument.containsKey("isstop")) {
    isstop = jsonDocument["isstop"];
  }
  else {
    isstop = 0;
  }

  if (jsonDocument.containsKey("isaccel")) {
    isaccel = jsonDocument["isaccel"];
  }
  else {
    isaccel = 1;
  }

  if (jsonDocument.containsKey("isen")) {
    isen = jsonDocument["isen"];
  }
  else {
    isen = 0;
  }

  if (jsonDocument.containsKey("isforever")) {
    isforever = jsonDocument["isforever"];
  }
  else {
    isforever = 0;
  }

  jsonDocument.clear();

  if (DEBUG) {
    Serial.print("speed0 "); Serial.println(mspeed0);
    Serial.print("speed1 "); Serial.println(mspeed1);
    Serial.print("speed2 "); Serial.println(mspeed2);
    Serial.print("speed3 "); Serial.println(mspeed3);
    Serial.print("position0 "); Serial.println(mposition0);
    Serial.print("position1 "); Serial.println(mposition1);
    Serial.print("position2 "); Serial.println(mposition2);
    Serial.print("position3 "); Serial.println(mposition3);
    Serial.print("isabs "); Serial.println(isabs);
    Serial.print("isen "); Serial.println(isen);
    Serial.print("isstop "); Serial.println(isstop);
    Serial.print("isaccel "); Serial.println(isaccel);
    Serial.print("isforever "); Serial.println(isforever);
  }

  if (isstop) {
    // Immediately stop the motor
    //stepper_A.stop();
    stepper_X.stop();
    stepper_Y.stop();
    stepper_Z.stop();
    isforever = 0;
    setEnableMotor(false);

    //POSITION_MOTOR_A = stepper_A.currentPosition();
    POSITION_MOTOR_X = stepper_X.currentPosition();
    POSITION_MOTOR_Y = stepper_Y.currentPosition();
    POSITION_MOTOR_Z = stepper_Z.currentPosition();

    jsonDocument["POSA"] = POSITION_MOTOR_A;
    jsonDocument["POSX"] = POSITION_MOTOR_X;
    jsonDocument["POSY"] = POSITION_MOTOR_Y;
    jsonDocument["POSZ"] = POSITION_MOTOR_Z;
    return;
  }

  // prepare motor to run
  setEnableMotor(true);
  //stepper_A.setSpeed(mspeed0);
  stepper_X.setSpeed(mspeed1);
  stepper_Y.setSpeed(mspeed2);
  stepper_Z.setSpeed(mspeed3);
  //stepper_A.setMaxSpeed(mspeed0);
  stepper_X.setMaxSpeed(mspeed1);
  stepper_Y.setMaxSpeed(mspeed2);
  stepper_Z.setMaxSpeed(mspeed3);


  if(not isforever){
  if (isabs) {
    // absolute position coordinates
    //stepper_A.moveTo(SIGN_A * mposition0);
    stepper_X.moveTo(SIGN_X * mposition1);
    stepper_Y.moveTo(SIGN_Y * mposition2);
    stepper_Z.moveTo(SIGN_Z * mposition3);
  }
  else {
    // relative position coordinates
    //stepper_A.move(SIGN_A * mposition0);
    stepper_X.move(SIGN_X * mposition1);
    stepper_Y.move(SIGN_Y * mposition2);
    stepper_Z.move(SIGN_Z * mposition3);
  }
  }
  
  if (DEBUG) Serial.println("Start rotation in background");

  //POSITION_MOTOR_A = stepper_A.currentPosition();
  POSITION_MOTOR_X = stepper_X.currentPosition();
  POSITION_MOTOR_Y = stepper_Y.currentPosition();
  POSITION_MOTOR_Z = stepper_Z.currentPosition();

  jsonDocument["POSA"] = POSITION_MOTOR_A;
  jsonDocument["POSX"] = POSITION_MOTOR_X;
  jsonDocument["POSY"] = POSITION_MOTOR_Y;
  jsonDocument["POSZ"] = POSITION_MOTOR_Z;
}

void setEnableMotor(bool enable) {
  isactive = enable;
  digitalWrite(ENABLE, !enable);
  motor_enable = enable;
}

bool getEnableMotor() {
  return motor_enable;
}

void motor_set_fct() {


  // default value handling
  int axis = -1;
  if (jsonDocument.containsKey("axis")) {
    int axis = jsonDocument["axis"];
  }

  int currentposition = NULL;
  if (jsonDocument.containsKey("currentposition")) {
    int currentposition = jsonDocument["currentposition"];
  }

  int maxspeed = NULL;
  if (jsonDocument.containsKey("maxspeed")) {
    int maxspeed = jsonDocument["maxspeed"];
  }

  int accel = NULL;
  if (jsonDocument.containsKey("accel")) {
    int accel = jsonDocument["accel"];
  }

  int pinstep = -1;
  if (jsonDocument.containsKey("pinstep")) {
    int pinstep = jsonDocument["pinstep"];
  }

  int pindir = -1;
  if (jsonDocument.containsKey("pindir")) {
    int pindir = jsonDocument["pindir"];
  }

  int isen = -1;
  if (jsonDocument.containsKey("isen")) {
    int isen = jsonDocument["isen"];
  }

  int sign = NULL;
  if (jsonDocument.containsKey("sign")) {
    int sign = jsonDocument["sign"];
  }

  // DEBUG printing
  if (DEBUG) {
    Serial.print("axis "); Serial.println(axis);
    Serial.print("currentposition "); Serial.println(currentposition);
    Serial.print("maxspeed "); Serial.println(maxspeed);
    Serial.print("pinstep "); Serial.println(pinstep);
    Serial.print("pindir "); Serial.println(pindir);
    Serial.print("isen "); Serial.println(isen);
    Serial.print("accel "); Serial.println(accel);
    Serial.print("isen: "); Serial.println(isen);
  }


  if (accel >=0) {
    if (DEBUG) Serial.print("accel "); Serial.println(accel);
    switch (axis) {
      case 1:
        MAX_ACCELERATION_X = accel;
        stepper_X.setAcceleration(MAX_ACCELERATION_X);
        break;
      case 2:
        MAX_ACCELERATION_Y = accel;
        stepper_Y.setAcceleration(MAX_ACCELERATION_Y);
        break;
      case 3:
        MAX_ACCELERATION_Z = accel;
        stepper_Z.setAcceleration(MAX_ACCELERATION_Z);
        break;
    }
  }

  if (sign != NULL) {
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


  if (currentposition != NULL) {
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
        stepper_X.setMaxSpeed(maxspeed);
        break;
      case 2:
        stepper_Y.setMaxSpeed(maxspeed);
        break;
      case 3:
        stepper_Z.setMaxSpeed(maxspeed); 
        break;
    }
  }
  if (pindir != 0 and pinstep != 0) {
    if (axis == 1) {
      STEP_X = pinstep;
      DIR_X = pindir;
      //A4988 stepper_X(FULLSTEPS_PER_REV_X, DIR_X, STEP_X, SLEEP, MS1, MS2, MS3); //stepper_X = AccelStepper(AccelStepper::DRIVER, STEP_X, DIR_X);
    }
    else if (axis == 2) {
      STEP_Y = pinstep;
      DIR_Y = pindir;
      //A4988 stepper_X(FULLSTEPS_PER_REV_Y, DIR_Y, STEP_Y, SLEEP, MS1, MS2, MS3); //stepper_Y = AccelStepper(AccelStepper::DRIVER, STEP_Y, DIR_Y);
    }
    else if (axis == 3) {
      STEP_Z = pinstep;
      DIR_Z = pindir;
      //A4988 stepper_X(FULLSTEPS_PER_REV_Z, DIR_Z, STEP_Z, SLEEP, MS1, MS2, MS3); //stepper_Z = AccelStepper(AccelStepper::DRIVER, STEP_Z, DIR_Z);
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

  int mmaxspeed = 0;
  int mposition = 0;
  int pinstep = 0;
  int pindir = 0;
  int sign = 0;
  int mspeed =0;

  switch (axis) {
    case 1:
      if (DEBUG) Serial.println("AXIS 1");
      mmaxspeed = stepper_X.maxSpeed();
      mspeed = stepper_X.speed();
      POSITION_MOTOR_X = stepper_X.currentPosition();
      mposition = POSITION_MOTOR_X;
      pinstep = STEP_X;
      pindir = DIR_X;
      sign = SIGN_X;
      break;
    case 2:
      if (DEBUG) Serial.println("AXIS 2");
      mmaxspeed = stepper_Y.maxSpeed();
      mspeed = stepper_Y.speed();
      POSITION_MOTOR_Y = stepper_Y.currentPosition();
      mposition = POSITION_MOTOR_Y;
      pinstep = STEP_Y;
      pindir = DIR_Y;
      sign = SIGN_Y;
      break;
    case 3:
      if (DEBUG) Serial.println("AXIS 3");
      mmaxspeed = stepper_Z.maxSpeed();
      mspeed = stepper_Z.speed();
      POSITION_MOTOR_Z = stepper_Z.currentPosition();
      mposition = POSITION_MOTOR_Z;
      pinstep = STEP_Z;
      pindir = DIR_Z;
      sign = SIGN_Z;
      break;
    default:
      break;
  }

  jsonDocument.clear();
  jsonDocument["position"] = mposition;
  jsonDocument["speed"] = mspeed;
  jsonDocument["maxspeed"] = mmaxspeed;
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
  setEnableMotor(true);
  Serial.println("Setting Up Motor A,X,Y,Z");
  //stepper_A.setMaxSpeed(MAX_VELOCITY_A);
  stepper_X.setMaxSpeed(MAX_VELOCITY_X);
  stepper_Y.setMaxSpeed(MAX_VELOCITY_Y);
  stepper_Z.setMaxSpeed(MAX_VELOCITY_Z);
  //stepper_A.setAcceleration(MAX_ACCELERATION_A);
  stepper_X.setAcceleration(MAX_ACCELERATION_X);
  stepper_Y.setAcceleration(MAX_ACCELERATION_Y);
  stepper_Z.setAcceleration(MAX_ACCELERATION_Z);
  stepper_X.enableOutputs();
  stepper_Y.enableOutputs();
  stepper_Z.enableOutputs();
  stepper_X.runToNewPosition(-100);
  stepper_X.runToNewPosition(100);
  stepper_Y.runToNewPosition(-100);
  stepper_Y.runToNewPosition(100);
  stepper_Z.runToNewPosition(-100);
  stepper_Z.runToNewPosition(100);
  stepper_X.setCurrentPosition(0);
  stepper_Y.setCurrentPosition(0);
  stepper_Z.setCurrentPosition(0);
  setEnableMotor(false);
}



bool drive_motor_background() {

  // update motor positions
  POSITION_MOTOR_X = stepper_X.currentPosition();
  POSITION_MOTOR_Y = stepper_Y.currentPosition();
  POSITION_MOTOR_Z = stepper_Z.currentPosition();
  
  // this function is called during every loop cycle
  if (isforever) {
    // run forever
    //stepper_A.runSpeed();
    // is this a bug? Otherwise the speed won't be set properly - seems like it is accelerating eventhough it shouldnt
    //stepper_A.setSpeed(mspeed0);
    stepper_X.setSpeed(mspeed1);
    stepper_Y.setSpeed(mspeed2);
    stepper_Z.setSpeed(mspeed3);
    // we have to at least set this. It will be recomputed or something?!
    stepper_X.setMaxSpeed(mspeed1);
    stepper_Y.setMaxSpeed(mspeed2);
    stepper_Z.setMaxSpeed(mspeed3);

    stepper_X.runSpeed();
    stepper_Y.runSpeed();
    stepper_Z.runSpeed();
  }
  else {
    // run at constant speed
    if (isaccel) {
      //stepper_A.run();
      stepper_X.run();
      stepper_Y.run();
      stepper_Z.run();
    }
    else {
      //stepper_A.runSpeedToPosition();
      stepper_X.runSpeedToPosition();
      stepper_Y.runSpeedToPosition();
      stepper_Z.runSpeedToPosition();
    }
  }

  // PROBLEM; is running wont work here!
  if ((stepper_X.distanceToGo() == 0) and (stepper_Y.distanceToGo() == 0) and (stepper_Z.distanceToGo() == 0 ) and not isforever) {
    if (not isen) {
      setEnableMotor(false);
    }
    isactive = false;
    return true;
  }
  isactive = true;
  return false; //never reached, but keeps compiler happy?
}


/*
   wrapper for HTTP requests
*/


#ifdef IS_WIFI
void motor_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  if (DEBUG) serializeJsonPretty(jsonDocument, Serial);
  motor_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void motor_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  if (DEBUG) serializeJsonPretty(jsonDocument, Serial);
  motor_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void motor_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  if (DEBUG) serializeJsonPretty(jsonDocument, Serial);
  motor_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
