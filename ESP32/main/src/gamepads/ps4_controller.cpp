#include "ps4_controller.h"
#include "../motor/FocusMotor.h"
#include "../led/led_controller.h"
#include "../state/State.h"
#include "../laser/LaserController.h"
#include "../analog/AnalogController.h"

PS4Controller pS4Controller;

gamepad::gamepad()
{
}

void gamepad::start()
{
    Serial.println("Connnecting to the PS4 controller, please please the magic round button in the center..");
    pS4Controller.attach(gamepad_onAttach);
    pS4Controller.begin("1a:2b:3c:01:01:01 - UNICAST!");
    pS4Controller.attachOnConnect(gamepad_onConnect);
    pS4Controller.attachOnDisconnect(gamepad_onDisConnect);
    const char*  PS4_MACADDESS = "1a:2b:3c:01:01:01";
    Serial.println(PS4_MACADDESS);
    Serial.println("PS4 controler is set up.");
}

void gamepad::onConnect() {
  if (DEBUG) Serial.println("PS4 Controller Connected.");
  IS_PSCONTROLER_ACTIVE = true;
  motor->setEnableMotor(true);

    if (pS4Controller.Charging())
    {
        Serial.println("The controller is charging");
    }

    Serial.printf("Battery Level : %d\n", pS4Controller.Battery());
    Serial.println();
}

void gamepad::onAttach() {
  pS4Controller.attach(gamepad_activate);
}



void gamepad::onDisConnect() {
  if (DEBUG) Serial.println("PS4 Controller Connected.");
  motor->setEnableMotor(false);
}



void gamepad::activate() {
  // callback for events
  if (pS4Controller.event.button_down.share) {
    IS_PSCONTROLER_ACTIVE = !IS_PSCONTROLER_ACTIVE;
    if (DEBUG) Serial.print("Setting manual mode to: ");
    if (DEBUG) Serial.println(IS_PSCONTROLER_ACTIVE);
    motor->setEnableMotor(IS_PSCONTROLER_ACTIVE);
    delay(1000); //Debounce?
  }
  if (pS4Controller.event.button_down.cross) {
    IS_PS_CONTROLER_LEDARRAY = !IS_PS_CONTROLER_LEDARRAY;
    if (DEBUG) Serial.print("Turning LED Matrix to (cross): ");
    if (DEBUG) Serial.println(IS_PS_CONTROLER_LEDARRAY);

    led->set_all(255*IS_PS_CONTROLER_LEDARRAY,255*IS_PS_CONTROLER_LEDARRAY,255*IS_PS_CONTROLER_LEDARRAY);
    delay(1000); //Debounce?
  }
  if (pS4Controller.event.button_down.circle) {
    IS_PS_CONTROLER_LEDARRAY = !IS_PS_CONTROLER_LEDARRAY;
    if (DEBUG) Serial.print("Turning LED Matrix to (circle): ");
    if (DEBUG) Serial.println(IS_PS_CONTROLER_LEDARRAY);

    led->set_center(255*IS_PS_CONTROLER_LEDARRAY,255*IS_PS_CONTROLER_LEDARRAY,255*IS_PS_CONTROLER_LEDARRAY);
    delay(1000); //Debounce?

  }
  // LASER
  if (pS4Controller.event.button_down.triangle) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(laser->PWM_CHANNEL_LASER_1, 10000);
    delay(100); //Debounce?
  }
  if (pS4Controller.event.button_down.square) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(laser->PWM_CHANNEL_LASER_1, 0);
    delay(100); //Debounce?
    }

// FOCUS
/*
  if (pS4Controller.event.button_down.up) {
    if (not getEnableMotor())
      setEnableMotor(true);
    POSITION_MOTOR_X = stepper_X.currentPosition();
    stepper_X.move(POSITION_MOTOR_X+2);
    delay(100); //Debounce?
  }
  if (pS4Controller.event.button_down.down) {
        if (not getEnableMotor())
      setEnableMotor(true);
    POSITION_MOTOR_X = stepper_X.currentPosition();
    stepper_X.move(POSITION_MOTOR_X-2);
    delay(100); //Debounce?
  }
*/

  // LASER 1
  if (pS4Controller.event.button_down.up) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(laser->PWM_CHANNEL_LASER_2, 20000);
    delay(100); //Debounce?
  }
  if (pS4Controller.event.button_down.down) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(laser->PWM_CHANNEL_LASER_2, 0);
    delay(100); //Debounce?
  }

  // LASER 2
  if (pS4Controller.event.button_down.right) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(laser->PWM_CHANNEL_LASER_1, 20000);
    delay(100); //Debounce?
  }
  if (pS4Controller.event.button_down.left) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(laser->PWM_CHANNEL_LASER_1, 0);
    delay(100); //Debounce?
  }
  
}



void gamepad::control() {
  if (pS4Controller.isConnected() and IS_PSCONTROLER_ACTIVE) {
      // Y-Direction
      if ( abs(pS4Controller.data.analog.stick.ly) > offset_val) {
        // move_z
        stick_ly = pS4Controller.data.analog.stick.ly;
        stick_ly = stick_ly - sgn(stick_ly) * offset_val;
        motor->mspeed2 =  stick_ly * 5 * global_speed;
        if (!motor->getEnableMotor())
          motor->setEnableMotor(true);
      }
      else if (motor->mspeed2 != 0) {
        motor->mspeed2 = 0;
        motor->stepper_Y->setSpeed(motor->mspeed2); // set motor off only once to not affect other modes
      }

      // Z-Direction
      if ( (abs(pS4Controller.data.analog.stick.rx) > offset_val)) {
        // move_x
        stick_rx = PS4.data.analog.stick.rx;
        stick_rx = stick_rx - sgn(stick_rx) * offset_val;
        motor->mspeed3  = stick_rx * 5 * global_speed;
        if (motor->getEnableMotor())
          motor->setEnableMotor(true);
      }
      else if (motor->mspeed3 != 0) {
        motor->mspeed3 = 0;
        motor->stepper_Z->setSpeed(motor->mspeed3); // set motor off only once to not affect other modes
      }

      // X-direction
      if ( (abs(pS4Controller.data.analog.stick.ry) > offset_val)) {
        // move_y
        stick_ry = pS4Controller.data.analog.stick.ry;
        stick_ry = stick_ry - sgn(stick_ry) * offset_val;
        motor->mspeed1 = stick_ry * 5 * global_speed;
        if (!motor->getEnableMotor())
          motor->setEnableMotor(true);
      }
      else if (motor->mspeed1 != 0) {
        motor->mspeed1 = 0;
        motor->stepper_X->setSpeed(motor->mspeed1); // set motor off only once to not affect other modes
      }

      /*
          // fine control for Z using buttons
          if ( PS4.data.button.down) {
            // fine focus +
            //run_motor(0, 0, 10);
            delay(100);
          }
          if ( PS4.data.button.up) {
            // fine focus -
            //run_motor(0, 0, -10);
            delay(100);
          }
      */


#ifdef IS_ANALOG
      /*
         Keypad left
      */
      if (pS4Controller.data.button.left) {
        // fine lens -
        analog_val_1 -= 1;
        delay(100);
        ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
      }
      if (pS4Controller.data.button.right) {
        // fine lens +
        analog_val_1 += 1;
        delay(100);
        ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
      }
      if ( pS4Controller.data.button.start) {
        // reset
        analog_val_1 = 0;
        ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
      }

      int offset_val_shoulder = 5;
      if ( abs(pS4Controller.data.analog.button.r2) > offset_val_shoulder) {
        // analog_val_1++ coarse
        if ((analog_val_1 + 1000 < pwm_max)) {
          analog_val_1 += 1000;
          ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
        }
        if (DEBUG) Serial.println(analog_val_1);
        delay(100);
      }

      if ( abs(pS4Controller.data.analog.button.l2) > offset_val_shoulder) {
        // analog_val_1-- coarse
        if ((analog_val_1 - 1000 > 0)) {
          analog_val_1 -= 1000;
          ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
        }
        if (DEBUG) Serial.println(analog_val_1);
        delay(100);
      }


      if ( abs(pS4Controller.data.analog.button.r1) > offset_val_shoulder) {
        // analog_val_1 + semi coarse
        if ((analog_val_1 + 100 < pwm_max)) {
          analog_val_1 += 100;
          ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
          delay(100);
        }
      }
      if ( abs(pS4Controller.data.analog.button.l1) > offset_val_shoulder) {
        // analog_val_1 - semi coarse
        if ((analog_val_1 - 100 > 0)) {
          analog_val_1 -= 100;
          ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
          delay(50);
        }
      }

#endif

      // run all motors simultaneously
      motor->stepper_X->setSpeed(motor->mspeed1);
      motor->stepper_Y->setSpeed(motor->mspeed2);
      motor->stepper_Z->setSpeed(motor->mspeed3);

      if (motor->mspeed1 || motor->mspeed2 || motor->mspeed3) {
        motor->isforever = true;
      }
      else {
        motor->isforever = false;
      }
    }
  
}
