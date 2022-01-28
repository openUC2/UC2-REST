/*
  This a simple example of the aREST Library for Arduino (Uno/Mega/Due/Teensy)
  using the Serial port. See the README file for more details.

  Written in 2014 by Marco Schwartz under a GPL license.
*/

// Libraries
#include <SPI.h>
#include <aREST.h>
#include <avr/wdt.h>

#include <ArduinoJson.h>
#include <AccelStepper.h>
#include "def_motorcontrol.h"


// Create aREST instance
aREST rest = aREST();

//Where the JSON for the current instruction lives
StaticJsonDocument<200> jsonDocument;
char buffer[200];


#define DEBUG 1
// https://www.pjrc.com/teensy/td_libs_AccelStepper.html
AccelStepper stepper_X = AccelStepper(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepper_Y = AccelStepper(AccelStepper::DRIVER, STEP_Y, DIR_Y);
AccelStepper stepper_Z = AccelStepper(AccelStepper::DRIVER, STEP_Z, DIR_Z);

int position_x = 0;
int position_y = 0;
int position_z = 0;

// Declare functions to be exposed to the API
int motor_act(String command);  // cause an action
char motor_get(); // get variable parameters
int motor_set(String command);  // set variable parameters
int ledControl(String command);

int test = 0;
void setup(void)
{
  
  // Start Serial
  Serial.begin(115200);
  Serial.println("Start");
  // Function to be exposed
  test = 24;
  rest.variable("test",&test);
 
  
  rest.function("motor_act", motor_act);
  rest.function("motor_get", motor_get);
  rest.function("motor_set", motor_set);
  rest.function("led",ledControl);
  
  // Give name and ID to device (ID should be 6 characters long)
  rest.set_id("2");
  rest.set_name("serial");

  // Start watchdog
  wdt_enable(WDTO_4S);


  /*

     Motor related settings

  */
  // Enable Motors
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);

  stepper_X.setMaxSpeed(MAX_VELOCITY_X_mm * steps_per_mm_X);
  stepper_Y.setMaxSpeed(MAX_VELOCITY_Y_mm * steps_per_mm_Y);
  stepper_Z.setMaxSpeed(MAX_VELOCITY_Z_mm * steps_per_mm_Z);

  stepper_X.setAcceleration(MAX_ACCELERATION_X_mm * steps_per_mm_X);
  stepper_Y.setAcceleration(MAX_ACCELERATION_Y_mm * steps_per_mm_Y);
  stepper_Z.setAcceleration(MAX_ACCELERATION_Z_mm * steps_per_mm_Z);

  stepper_X.enableOutputs();
  stepper_Y.enableOutputs();
  stepper_Z.enableOutputs();

  Serial.println("Start 2");
}




void loop() {

  // Handle REST calls
  rest.handle(Serial);
  wdt_reset();

}


// Custom function accessible by the API
int motor_act(String command) {
  Serial.println("motor_act");

  DeserializationError error = deserializeJson(jsonDocument, command);
  if (error) {
    return 0;
  }
  int axis = jsonDocument["axis"];
  int mspeed = jsonDocument["speed"];
  long mposition = jsonDocument["position"];
  int isabsolute = jsonDocument["isabsolute"];

  if (DEBUG) {
    Serial.print("Axis "); Serial.println(axis);
    Serial.print("Speed "); Serial.println(mspeed);
    Serial.print("Position "); Serial.println(mposition);
    Serial.print("isabsolute "); Serial.println(isabsolute);
  }

  switch (axis) {
    case 1:
      if (isabsolute) stepper_X.moveTo(mposition * steps_per_mm_X);
      else stepper_X.move(mposition * steps_per_mm_X);
      stepper_X.setSpeed(mspeed * steps_per_mm_X);
      stepper_X.runToPosition();
      break;
    case 2:
      if (isabsolute) stepper_Y.moveTo(mposition * steps_per_mm_Y);
      else stepper_Y.moveTo(mposition * steps_per_mm_Y);
      stepper_Y.setSpeed(mspeed * steps_per_mm_Y);
      stepper_Y.runToPosition();
      break;
    case 3:
      if (isabsolute) stepper_Z.moveTo(mposition * steps_per_mm_Z);
      else stepper_Z.move(mposition * steps_per_mm_Z);
      stepper_Z.setSpeed(mspeed * steps_per_mm_Z);
      stepper_Z.runToPosition();
    default:
      // Statement(s)
      break;
  }
  return "1";
}

int motor_set(String command) {
  DeserializationError error = deserializeJson(jsonDocument, command);
  if (error) {
    return 0;
  }

  int axis = jsonDocument["axis"];
  String task = jsonDocument["task"];


  if (task == "currentposition") {
    int currentposition = jsonDocument["currentposition"];
    switch (axis) {
      case 1:
        stepper_X.setCurrentPosition(currentposition);
      case 2:
        stepper_Y.setCurrentPosition(currentposition);
      case 3:
        stepper_Z.setCurrentPosition(currentposition);
    }
  }
  else if (task == "maxspeed") {
    int maxspeed = jsonDocument["maxspeed"];
    switch (axis) {
      case 1:
        stepper_X.setMaxSpeed(maxspeed);
      case 2:
        stepper_Y.setMaxSpeed(maxspeed);
      case 3:
        stepper_Z.setMaxSpeed(maxspeed);
    }
  }
  else if (task == "acceleration") {
    int acceleration = jsonDocument["acceleration"];
    switch (axis) {
      case 1:
        stepper_X.setAcceleration(acceleration);
      case 2:
        stepper_Y.setAcceleration(acceleration);
      case 3:
        stepper_Z.setAcceleration(acceleration);
    }
  }
  else if (task == "motorconfig#") {
    int pinstep = jsonDocument["pinstep"];
    int pindir = jsonDocument["pindir"];
    switch (axis) {
      case 1:
        STEP_X = pinstep;
        DIR_X = pindir;
        stepper_X = AccelStepper(AccelStepper::DRIVER, STEP_X, DIR_X);
      case 2:
        STEP_Y = pinstep;
        DIR_Y = pindir;
        stepper_Y = AccelStepper(AccelStepper::DRIVER, STEP_Y, DIR_Y);
      case 3:
        STEP_Z = pinstep;
        DIR_Z = pindir;
        stepper_Z = AccelStepper(AccelStepper::DRIVER, STEP_Z, DIR_Z);
    }
  }
  else if (task == "stop") {
    switch (axis) {
      case 1:
        stepper_X.stop();
      case 2:
        stepper_Y.stop();
      case 3:
        stepper_Z.stop();
    }
  }
}





// Custom function accessible by the API
char motor_get() {

  int axis = 1; jsonDocument["axis"];

  int mmaxspeed = 0;
  int mspeed = 0;
  int mposition = 0;
  int pinstep = 0;
  int pindir = 0;

  switch (axis) {
    case 1:
      mmaxspeed = stepper_X.maxSpeed();
      mspeed = stepper_X.speed();
      mposition = stepper_X.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
    case 2:
      mmaxspeed = stepper_Y.maxSpeed();
      mspeed = stepper_Y.speed();
      mposition = stepper_Y.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
    case 3:
      mmaxspeed = stepper_Z.maxSpeed();
      mspeed = stepper_Z.speed();
      mposition = stepper_Z.currentPosition();
      pinstep = STEP_X;
      pindir = DIR_X;
  }

  jsonDocument.clear();
  jsonDocument["position"] = mposition;
  jsonDocument["speed"] = mspeed;
  jsonDocument["maxspeed"] = mmaxspeed;
  serializeJson(jsonDocument, buffer);
  return buffer;
}


// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(6,state);
  return 1;
}
