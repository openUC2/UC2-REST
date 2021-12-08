// includes
#include "A4988.h"
#include <Stepper.h>

// const definitions
const int MOTOR_STEPS = 200;
const int PIN_DIR = 1;
const int PIN_STEP = 2; 
const int SLEEP = 0;
const int MS1 = 0;
const int MS2 = 0;
const int MS3 = 0; 
const int ENABLE = 3;

// variable definitions
int STEPS = 200;
int SPEED = 0;
boolean is_motor_running = false;
boolean is_motor_on = false;


// special variable definitions 
int glob_motor_steps[] = {0, 0, 0};

A4988 stepper();

A4988 stepper(MOTOR_STEPS, PIN_DIR, PIN_STEP, SLEEP, MS1, MS2, MS3);


// setup
setup(){

    // enable motors
    pinMode(ENABLE, OUTPUT);
    digitalWrite(ENABLE, LOW);

    Serial.println("Initialize Stepper");
    stepper_x.begin(RPM);
    stepper_x.enable();
    stepper_x.setMicrostep(1);  // Set microstep mode to 1:1
    stepper_x.rotate(360);     // forward revolution
    stepper_x.rotate(-360);    // reverse revolution


    // register motors
    Serial.println("Start runStepperTask");
    xTaskCreatePinnedToCore(
      runStepperTask,             // Task function.
      "runStepperTask",           // String with name of task.
      10000,                   // Stack size in words.
      (void*)&glob_motor_steps,      // Parameter passed as input of the task
      1,                         // Priority of the task.
      NULL,                     // Task handle.
      0);                       // core #
}

// loop
loop(){
     // do nothing
}

// special function
void runStepperTask( void * param) {
  uint8_t* localparameters = (uint8_t*)param;
  // TODO: Handle three axis?
  while (is_wifi) {
    if (abs(localparameters[0]) > 0) {
      stepper.move(localparameters[0]);
    }
    if (abs(localparameters[1]) > 0) {
      stepper.move(localparameters[1]);
    }
    if (abs(localparameters[2]) > 0) {
      stepper.move(localparameters[2]);
    }
  }
  vTaskDelete( NULL );
}


// functions
void move_http() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int steps = jsonDocument["steps"];
  int speed = jsonDocument["speed"];
  run_motor(steps, speed);
  server.send(200, "application/json", "{}");  
}

void run_motor(int steps, int speed) {
  if (steps == 0) {
    // run in background (non blocking!)
    glob_motor_steps[0] = sgn(speed) * 10;
  }
  else {
    // run in foreground (blocking!)
    glob_motor_steps[0] = 0;
    stepper.begin(abs(speed));
    stepper.rotate(steps);
  }
}