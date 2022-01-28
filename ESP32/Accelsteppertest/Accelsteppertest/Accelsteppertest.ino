#include <AccelStepper.h>
#include "def_motorcontrol.h"


AccelStepper stepper_X = AccelStepper(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepper_Y = AccelStepper(AccelStepper::DRIVER, STEP_Y, DIR_Y);
AccelStepper stepper_Z = AccelStepper(AccelStepper::DRIVER, STEP_Z, DIR_Z);



void setup() {
  Serial.begin(115200);
  // Enable Motors
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);

  stepper_X.setMaxSpeed(MAX_VELOCITY_X_mm * steps_per_mm_X);
  stepper_Y.setMaxSpeed(MAX_VELOCITY_Y_mm * steps_per_mm_Y);
  stepper_Z.setMaxSpeed(MAX_VELOCITY_Z_mm * steps_per_mm_Z);
  Serial.print("Speed ");
  Serial.println(MAX_VELOCITY_X_mm * steps_per_mm_X);
  
  stepper_X.setAcceleration(MAX_ACCELERATION_X_mm * steps_per_mm_X);
  stepper_Y.setAcceleration(MAX_ACCELERATION_Y_mm * steps_per_mm_Y);
  stepper_Z.setAcceleration(MAX_ACCELERATION_Z_mm * steps_per_mm_Z);

  Serial.print("Accel ");
  Serial.println(MAX_ACCELERATION_X_mm * steps_per_mm_X);
  stepper_X.enableOutputs();
  stepper_Y.enableOutputs();
  stepper_Z.enableOutputs();

}

int relative_position_X = 3*steps_per_mm_X;
int relative_position_Y = 3*steps_per_mm_Y;
int relative_position_Z = 3*steps_per_mm_Z;

void loop() {
   stepper_X.moveTo(relative_position_X);
  stepper_X.runToPosition();
  stepper_Y.moveTo(relative_position_Y);  
  stepper_Y.runToPosition();
  stepper_Z.moveTo(relative_position_Z);  
  stepper_Z.runToPosition();
    
  stepper_X.moveTo(0);
  stepper_X.runToPosition();
  stepper_Y.moveTo(0);  
  stepper_Y.runToPosition();
  stepper_Z.moveTo(0);  
  stepper_Z.runToPosition();
  
}
