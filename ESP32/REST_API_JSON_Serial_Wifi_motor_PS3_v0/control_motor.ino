#ifdef IS_MOTOR

// Custom function accessible by the API
void motor_act_fct() {
  if (DEBUG) Serial.println("motor_act_fct");


  // assign default values to thhe variables

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
    Serial.print("speed1 "); Serial.println(mspeed1);
    Serial.print("speed2 "); Serial.println(mspeed2);
    Serial.print("speed3 "); Serial.println(mspeed3);
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
  stepper_X.setSpeed(mspeed1);
  stepper_X.setMaxSpeed(mspeed1);
  stepper_Y.setSpeed(mspeed2);
  stepper_Y.setMaxSpeed(mspeed2);
  stepper_Z.setSpeed(mspeed3);
  stepper_Z.setMaxSpeed(mspeed3);

  if (isabs) {
    stepper_X.moveTo(SIGN_X * mposition1);
    stepper_Y.moveTo(SIGN_Y * mposition2);
    stepper_Z.moveTo(SIGN_Z * mposition3);
  }  
  else{
    stepper_X.move(SIGN_X * mposition1);
    stepper_Y.move(SIGN_Y * mposition2);
    stepper_Z.move(SIGN_Z * mposition3);     
  }



  if (isblock) {
    if (DEBUG) Serial.println("Start rotation");
    while (!drive_motor_background()) {
      }
    if (DEBUG) Serial.println("Done with rotation");
  }
  else {
    if (DEBUG) Serial.println("Start rotation in background");
  }

  // TODO: not true for non-blocking!
  POSITION_MOTOR_X = stepper_X.currentPosition();
  POSITION_MOTOR_Y = stepper_Y.currentPosition();
  POSITION_MOTOR_Z = stepper_Z.currentPosition();

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

  /*
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
  */
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
  Serial.println("Setting Up Motor X,Y,Z");
  stepper_X.setMaxSpeed(MAX_VELOCITY_X);
  stepper_Y.setMaxSpeed(MAX_VELOCITY_Y);
  stepper_Z.setMaxSpeed(MAX_VELOCITY_Z);
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
  digitalWrite(ENABLE, HIGH);
}


bool drive_motor_background() {

  if (is_accel){
  stepper_X.run();
  stepper_Y.run();
  stepper_Z.run();
  }
  else{
    // run at constant speed
  stepper_X.runSpeed();
  stepper_Y.runSpeed();
  stepper_Z.runSpeed();    
  }
  
  if (not stepper_X.isRunning() and not stepper_Y.isRunning() and not stepper_Z.isRunning()) {
    if (DEBUG) Serial.println("Shutting down motor motion");
    if (not isen) {
      digitalWrite(ENABLE, HIGH);
    }
    isblock = true;

    return true;
  }
  else {
    if(isblock) return false;
  }
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
