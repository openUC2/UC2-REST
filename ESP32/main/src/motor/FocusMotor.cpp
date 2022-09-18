#include "FocusMotor.h"
#include <ArduinoJson.h>

FocusMotor::FocusMotor()
{
};
FocusMotor::~FocusMotor()
{
};

void FocusMotor::act()
{
    if (DEBUG) Serial.println("motor_act_fct");


  // assign default values to thhe variables
  if (WifiController::getJDoc()->containsKey("speed0")) {
    mspeed0 = (*WifiController::getJDoc())["speed0"];
  }
  else if (WifiController::getJDoc()->containsKey("speed")) {
    mspeed0 = (*WifiController::getJDoc())["speed"];
  }
  if (WifiController::getJDoc()->containsKey("speed1")) {
    mspeed1 = (*WifiController::getJDoc())["speed1"];
  }
  else if (WifiController::getJDoc()->containsKey("speed")) {
    mspeed1 = (*WifiController::getJDoc())["speed"];
  }
  if (WifiController::getJDoc()->containsKey("speed2")) {
    mspeed2 = (*WifiController::getJDoc())["speed2"];
  }
  else if (WifiController::getJDoc()->containsKey("speed")) {
    mspeed2 = (*WifiController::getJDoc())["speed"];
  }
  if (WifiController::getJDoc()->containsKey("speed3")) {
    mspeed3 = (*WifiController::getJDoc())["speed3"];
  }
  else if (WifiController::getJDoc()->containsKey("speed")) {
    mspeed3 = (*WifiController::getJDoc())["speed"];
  }

  if (WifiController::getJDoc()->containsKey("pos0")) {
    mposition0 = (*WifiController::getJDoc())["pos0"];
  }
  else {
    mposition0 = 0;
  }
  if (WifiController::getJDoc()->containsKey("pos1")) {
    mposition1 = (*WifiController::getJDoc())["pos1"];
  }
  else {
    mposition1 = 0;
  }

  if (WifiController::getJDoc()->containsKey("pos2")) {
    mposition2 = (*WifiController::getJDoc())["pos2"];
  }
  else {
    mposition2 = 0;
  }

  if (WifiController::getJDoc()->containsKey("pos3")) {
    mposition3 = (*WifiController::getJDoc())["pos3"];
  }
  else {
    mposition3 = 0;
  }

  if (WifiController::getJDoc()->containsKey("isabs")) {
    isabs = (*WifiController::getJDoc())["isabs"];
  }
  else {
    isabs = 0;
  }

  if (WifiController::getJDoc()->containsKey("isstop")) {
    isstop = (*WifiController::getJDoc())["isstop"];
  }
  else {
    isstop = 0;
  }

  if (WifiController::getJDoc()->containsKey("isaccel")) {
    isaccel = (*WifiController::getJDoc())["isaccel"];
  }
  else {
    isaccel = 1;
  }

  if (WifiController::getJDoc()->containsKey("isen")) {
    isen = (*WifiController::getJDoc())["isen"];
  }
  else {
    isen = 0;
  }

  if (WifiController::getJDoc()->containsKey("isforever")) {
    isforever = (*WifiController::getJDoc())["isforever"];
  }
  else {
    isforever = 0;
  }

  WifiController::getJDoc()->clear();

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
    stepper_A->stop();
    stepper_X->stop();
    stepper_Y->stop();
    stepper_Z->stop();
    isforever = 0;
    setEnableMotor(false);

    POSITION_MOTOR_A = stepper_A->currentPosition();
    POSITION_MOTOR_X = stepper_X->currentPosition();
    POSITION_MOTOR_Y = stepper_Y->currentPosition();
    POSITION_MOTOR_Z = stepper_Z->currentPosition();

    (*WifiController::getJDoc())["POSA"] = POSITION_MOTOR_A;
    (*WifiController::getJDoc())["POSX"] = POSITION_MOTOR_X;
    (*WifiController::getJDoc())["POSY"] = POSITION_MOTOR_Y;
    (*WifiController::getJDoc())["POSZ"] = POSITION_MOTOR_Z;
    return;
  }


#if defined IS_PS3 || defined IS_PS4  
  if(ps_c.IS_PSCONTROLER_ACTIVE)
    ps_c.IS_PSCONTROLER_ACTIVE=false; // override PS controller settings #TODO: Somehow reset it later?
#endif
  // prepare motor to run
  setEnableMotor(true);
  stepper_A->setSpeed(mspeed0);
  stepper_X->setSpeed(mspeed1);
  stepper_Y->setSpeed(mspeed2);
  stepper_Z->setSpeed(mspeed3);
  stepper_A->setMaxSpeed(mspeed0);
  stepper_X->setMaxSpeed(mspeed1);
  stepper_Y->setMaxSpeed(mspeed2);
  stepper_Z->setMaxSpeed(mspeed3);

  
  if(!isforever){
  if (isabs) {
    // absolute position coordinates
    stepper_A->moveTo(SIGN_A * mposition0);
    stepper_X->moveTo(SIGN_X * mposition1);
    stepper_Y->moveTo(SIGN_Y * mposition2);
    stepper_Z->moveTo(SIGN_Z * mposition3);
  }
  else {
    // relative position coordinates
    stepper_A->move(SIGN_A * mposition0);
    stepper_X->move(SIGN_X * mposition1);
    stepper_Y->move(SIGN_Y * mposition2);
    stepper_Z->move(SIGN_Z * mposition3);
  }
  }
  
  if (DEBUG) Serial.println("Start rotation in background");

  POSITION_MOTOR_A = stepper_A->currentPosition();
  POSITION_MOTOR_X = stepper_X->currentPosition();
  POSITION_MOTOR_Y = stepper_Y->currentPosition();
  POSITION_MOTOR_Z = stepper_Z->currentPosition();

  (*WifiController::getJDoc())["POSA"] = POSITION_MOTOR_A;
  (*WifiController::getJDoc())["POSX"] = POSITION_MOTOR_X;
  (*WifiController::getJDoc())["POSY"] = POSITION_MOTOR_Y;
  (*WifiController::getJDoc())["POSZ"] = POSITION_MOTOR_Z;
}

void FocusMotor::setEnableMotor(bool enable)
{
    isBusy = enable;
    digitalWrite(pins->ENABLE, !enable);
    motor_enable = enable;
}

bool FocusMotor::getEnableMotor()
{
    return motor_enable;
}

void FocusMotor::set()
{
    // default value handling
  int axis = -1;
  if (WifiController::getJDoc()->containsKey("axis")) {
    axis = (*WifiController::getJDoc())["axis"];
  }

  int currentposition = NULL;
  if (WifiController::getJDoc()->containsKey("currentposition")) {
    currentposition = (*WifiController::getJDoc())["currentposition"];
  }

  int maxspeed = NULL;
  if (WifiController::getJDoc()->containsKey("maxspeed")) {
    maxspeed = (*WifiController::getJDoc())["maxspeed"];
  }

  int accel = NULL;
  if (WifiController::getJDoc()->containsKey("accel")) {
    accel = (*WifiController::getJDoc())["accel"];
  }

  int pinstep = -1;
  if (WifiController::getJDoc()->containsKey("pinstep")) {
    pinstep = (*WifiController::getJDoc())["pinstep"];
  }

  int pindir = -1;
  if (WifiController::getJDoc()->containsKey("pindir")) {
    pindir = (*WifiController::getJDoc())["pindir"];
  }

  int isen = -1;
  if (WifiController::getJDoc()->containsKey("isen")) {
    isen = (*WifiController::getJDoc())["isen"];
  }

  int sign = NULL;
  if (WifiController::getJDoc()->containsKey("sign")) {
    sign = (*WifiController::getJDoc())["sign"];
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
      case 0:
        MAX_ACCELERATION_A = accel;
        stepper_X->setAcceleration(MAX_ACCELERATION_A);
        break;      
      case 1:
        MAX_ACCELERATION_X = accel;
        stepper_X->setAcceleration(MAX_ACCELERATION_X);
        break;
      case 2:
        MAX_ACCELERATION_Y = accel;
        stepper_Y->setAcceleration(MAX_ACCELERATION_Y);
        break;
      case 3:
        MAX_ACCELERATION_Z = accel;
        stepper_Z->setAcceleration(MAX_ACCELERATION_Z);
        break;
    }
  }

  if (sign != NULL) {
    if (DEBUG) Serial.print("sign "); Serial.println(sign);
    switch (axis) {
      case 0:
        SIGN_A = sign;
        break;
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
      case 0:
        POSITION_MOTOR_A = currentposition;
        break;
      case 1:
        POSITION_MOTOR_X = currentposition; 
        break;
      case 2:
        POSITION_MOTOR_Y = currentposition; 
      case 3:
        POSITION_MOTOR_Z = currentposition; 
        break;
    }
  }
  if (maxspeed != 0) {
    switch (axis) {
      case 1:
        stepper_X->setMaxSpeed(maxspeed);
        break;
      case 2:
        stepper_Y->setMaxSpeed(maxspeed);
        break;
      case 3:
        stepper_Z->setMaxSpeed(maxspeed); 
        break;
    }
  }
  if (pindir != 0 and pinstep != 0) {
    if (axis == 0) {
      pins->STEP_A = pinstep;
      pins->DIR_A = pindir;
    }
    else if (axis == 1) {
      pins->STEP_X = pinstep;
      pins->DIR_X = pindir;
    }
    else if (axis == 2) {
      pins->STEP_Y = pinstep;
      pins->DIR_Y = pindir;
   }
    else if (axis == 3) {
      pins->STEP_Z = pinstep;
      pins->DIR_Z = pindir;
    }
  }

  //if (DEBUG) Serial.print("isen "); Serial.println(isen);
  if (isen != 0 and isen) {
    digitalWrite(pins->ENABLE, 0);
  }
  else if (isen != 0 and not isen) {
    digitalWrite(pins->ENABLE, 1);
  }
  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["return"] = 1;
}

void FocusMotor::get()
{
    int axis = (*WifiController::getJDoc())["axis"];
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
      mmaxspeed = stepper_X->maxSpeed();
      mspeed = stepper_X->speed();
      POSITION_MOTOR_X = stepper_X->currentPosition();
      mposition = POSITION_MOTOR_X;
      pinstep = pins->STEP_X;
      pindir = pins->DIR_X;
      sign = SIGN_X;
      break;
    case 2:
      if (DEBUG) Serial.println("AXIS 2");
      mmaxspeed = stepper_Y->maxSpeed();
      mspeed = stepper_Y->speed();
      POSITION_MOTOR_Y = stepper_Y->currentPosition();
      mposition = POSITION_MOTOR_Y;
      pinstep = pins->STEP_Y;
      pindir = pins->DIR_Y;
      sign = SIGN_Y;
      break;
    case 3:
      if (DEBUG) Serial.println("AXIS 3");
      mmaxspeed = stepper_Z->maxSpeed();
      mspeed = stepper_Z->speed();
      POSITION_MOTOR_Z = stepper_Z->currentPosition();
      mposition = POSITION_MOTOR_Z;
      pinstep = pins->STEP_Z;
      pindir = pins->DIR_Z;
      sign = SIGN_Z;
      break;
    default:
      break;
  }

  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["position"] = mposition;
  (*WifiController::getJDoc())["speed"] = mspeed;
  (*WifiController::getJDoc())["maxspeed"] = mmaxspeed;
  (*WifiController::getJDoc())["pinstep"] = pinstep;
  (*WifiController::getJDoc())["pindir"] = pindir;
  (*WifiController::getJDoc())["sign"] = sign;
}

void FocusMotor::setup(PINDEF * pins)
{
    this->pins = pins;
    stepper_A = new AccelStepper(AccelStepper::DRIVER, pins->STEP_A, pins->DIR_A);
    stepper_X = new AccelStepper(AccelStepper::DRIVER, pins->STEP_X, pins->DIR_X);
    stepper_Y = new AccelStepper(AccelStepper::DRIVER, pins->STEP_Y, pins->DIR_Y);
    stepper_Z = new AccelStepper(AccelStepper::DRIVER, pins->STEP_Z, pins->DIR_Z);
  /*
     Motor related settings
  */
  Serial.println("Setting Up Motors");
  pinMode(pins->ENABLE, OUTPUT);
  setEnableMotor(true);
  Serial.println("Setting Up Motor A,X,Y,Z");
  stepper_A->setMaxSpeed(MAX_VELOCITY_A);
  stepper_X->setMaxSpeed(MAX_VELOCITY_X);
  stepper_Y->setMaxSpeed(MAX_VELOCITY_Y);
  stepper_Z->setMaxSpeed(MAX_VELOCITY_Z);
  stepper_A->setAcceleration(MAX_ACCELERATION_A);
  stepper_X->setAcceleration(MAX_ACCELERATION_X);
  stepper_Y->setAcceleration(MAX_ACCELERATION_Y);
  stepper_Z->setAcceleration(MAX_ACCELERATION_Z);
  stepper_X->enableOutputs();
  stepper_Y->enableOutputs();
  stepper_Z->enableOutputs();
  stepper_X->runToNewPosition(-100);
  stepper_X->runToNewPosition(100);
  stepper_Y->runToNewPosition(-100);
  stepper_Y->runToNewPosition(100);
  stepper_Z->runToNewPosition(-100);
  stepper_Z->runToNewPosition(100);
  stepper_X->setCurrentPosition(0);
  stepper_Y->setCurrentPosition(0);
  stepper_Z->setCurrentPosition(0);
  setEnableMotor(false);
}

bool FocusMotor::background()
{
    
  // update motor positions
  POSITION_MOTOR_A = stepper_A->currentPosition();
  POSITION_MOTOR_X = stepper_X->currentPosition();
  POSITION_MOTOR_Y = stepper_Y->currentPosition();
  POSITION_MOTOR_Z = stepper_Z->currentPosition();
  
  // this function is called during every loop cycle
  if (isforever) {
    // run forever
    // is this a bug? Otherwise the speed won't be set properly - seems like it is accelerating eventhough it shouldnt
    stepper_A->setSpeed(mspeed0);
    stepper_X->setSpeed(mspeed1);
    stepper_Y->setSpeed(mspeed2);
    stepper_Z->setSpeed(mspeed3);
    // we have to at least set this. It will be recomputed or something?!
    stepper_A->setMaxSpeed(mspeed1);
    stepper_X->setMaxSpeed(mspeed1);
    stepper_Y->setMaxSpeed(mspeed2);
    stepper_Z->setMaxSpeed(mspeed3);

    stepper_A->runSpeed();
    stepper_X->runSpeed();
    stepper_Y->runSpeed();
    stepper_Z->runSpeed();
  }
  else {
    // run at constant speed
    if (isaccel) {
      stepper_A->run();
      stepper_X->run();
      stepper_Y->run();
      stepper_Z->run();
    }
    else {
      stepper_A->runSpeedToPosition();
      stepper_X->runSpeedToPosition();
      stepper_Y->runSpeedToPosition();
      stepper_Z->runSpeedToPosition();
    }
  }

  // PROBLEM; is running wont work here!
  if ((stepper_A->distanceToGo() == 0) && (stepper_X->distanceToGo() == 0) && (stepper_Y->distanceToGo() == 0) && (stepper_Z->distanceToGo() == 0 ) && !isforever) {
    if (not isen) {
      setEnableMotor(false);
    }
    isBusy = false;
    return true;
  }
  isBusy = true;
  return false; //never reached, but keeps compiler happy?
}
