#ifdef IS_PS3
#include "ps3_controller.h"

void ps3_controller::start()
{
    Serial.println("Connnecting to the PS3 controller, please please the magic round button in the center..");
    Ps3.attach(ps3_onAttach);
    Ps3.attachOnConnect(ps3_onConnect);
    Ps3.attachOnDisconnect(ps3_onDisConnect);
    const char* PS3_MACADDESS = "01:02:03:04:05:06";
    Ps3.begin("01:02:03:04:05:06");
    Serial.println(PS3_MACADDESS);
    //String address = Ps3.getAddress(); // have arbitrary address?
    //Serial.println(address);
    Serial.println("PS3 controler is set up.");
}

void ps3_controller::onConnect()
{
    if (DEBUG) Serial.println("PS3 Controller Connected.");
    IS_PSCONTROLER_ACTIVE = true;
    motor->setEnableMotor(true);
}

void ps3_controller::onAttach() {
  Ps3.attach(ps3_activate);
}


void ps3_controller::onDisConnect() {
  if (DEBUG) Serial.println("PS3 Controller Connected.");
  motor->setEnableMotor(false);
}


void ps3_controller::activate() {
  // callback for events
  if (Ps3.event.button_down.select) {
    IS_PSCONTROLER_ACTIVE = !IS_PSCONTROLER_ACTIVE;
    if (DEBUG) Serial.print("Setting manual mode to: ");
    if (DEBUG) Serial.println(IS_PSCONTROLER_ACTIVE);
    motor->setEnableMotor(IS_PSCONTROLER_ACTIVE);
    delay(1000); //Debounce?
  }
  if (Ps3.event.button_down.cross) {
    IS_PS_CONTROLER_LEDARRAY = !IS_PS_CONTROLER_LEDARRAY;
    if (DEBUG) Serial.print("Turning LED Matrix to: ");
    if (DEBUG) Serial.println(IS_PS_CONTROLER_LEDARRAY);
    led->set_all(255 * IS_PS_CONTROLER_LEDARRAY, 255 * IS_PS_CONTROLER_LEDARRAY, 255 * IS_PS_CONTROLER_LEDARRAY);
    delay(1000); //Debounce?
  }


  // LASER 1
  if (Ps3.event.button_down.up) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(laser->PWM_CHANNEL_LASER_2, 20000);
    delay(100); //Debounce?
  }
  if (Ps3.event.button_down.down) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(laser->PWM_CHANNEL_LASER_2, 0);
    delay(100); //Debounce?
  }

  // LASER 2
  if (Ps3.event.button_down.right) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(laser->PWM_CHANNEL_LASER_1, 20000);
    delay(100); //Debounce?
  }
  if (Ps3.event.button_down.left) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(laser->PWM_CHANNEL_LASER_1, 0);
    delay(100); //Debounce?
  }


}

void ps3_controller::control() {
  if (Ps3.isConnected() and IS_PSCONTROLER_ACTIVE) {
    // Y-Direction
    if ( abs(Ps3.data.analog.stick.ly) > offset_val) {
      // move_z
      stick_ly = Ps3.data.analog.stick.ly;
      stick_ly = stick_ly - sgn(stick_ly) * offset_val;
      if (abs(stick_ly) > 100)
        stick_ly *= 2;

      motor->mspeed3 =  stick_ly * 5 * global_speed;
      if (!motor->getEnableMotor())
        motor->setEnableMotor(true);
    }
    else if (motor->mspeed3 != 0) {
      motor->mspeed3 = 0;
      motor->stepper_Y->setSpeed(motor->mspeed3); // set motor off only once to not affect other modes
    }

    // Z-Direction
    if ( (abs(Ps3.data.analog.stick.rx) > offset_val)) {
      // move_x
      stick_rx = Ps3.data.analog.stick.rx;
      stick_rx = stick_rx - sgn(stick_rx) * offset_val;

      if (abs(stick_rx) > 100)
        stick_rx *= 2;

      motor->mspeed2  = stick_rx * 5 * global_speed;
      if (!motor->getEnableMotor())
        motor->setEnableMotor(true);
    }
    else if (motor->mspeed2 != 0) {
      motor->mspeed2 = 0;
      motor->stepper_Z->setSpeed(motor->mspeed2); // set motor off only once to not affect other modes
    }

    // X-direction
    if ( (abs(Ps3.data.analog.stick.ry) > offset_val)) {
      // move_y
      stick_ry = Ps3.data.analog.stick.ry;
      stick_ry = stick_ry - sgn(stick_ry) * offset_val;
      if (abs(stick_ry) > 100)
        stick_ry *= 2;
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
        if ( Ps3.data.button.down) {
          // fine focus +
          //run_motor(0, 0, 10);
          delay(100);
        }
        if ( Ps3.data.button.up) {
          // fine focus -
          //run_motor(0, 0, -10);
          delay(100);
        }
    */


#ifdef IS_ANALOG
    /*
       Keypad left
    */
    if ( Ps3.data.button.left) {
      // fine lens -
      analog_val_1 -= 1;
      delay(100);
      ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
    }
    if ( Ps3.data.button.right) {
      // fine lens +
      analog_val_1 += 1;
      delay(100);
      ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
    }
    if ( Ps3.data.button.start) {
      // reset
      analog_val_1 = 0;
      ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
    }

    int offset_val_shoulder = 5;
    if ( abs(Ps3.data.analog.button.r2) > offset_val_shoulder) {
      // analog_val_1++ coarse
      if ((analog_val_1 + 1000 < pwm_max)) {
        analog_val_1 += 1000;
        ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
      }
      if (DEBUG) Serial.println(analog_val_1);
      delay(100);
    }

    if ( abs(Ps3.data.analog.button.l2) > offset_val_shoulder) {
      // analog_val_1-- coarse
      if ((analog_val_1 - 1000 > 0)) {
        analog_val_1 -= 1000;
        ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
      }
      if (DEBUG) Serial.println(analog_val_1);
      delay(100);
    }


    if ( abs(Ps3.data.analog.button.r1) > offset_val_shoulder) {
      // analog_val_1 + semi coarse
      if ((analog_val_1 + 100 < pwm_max)) {
        analog_val_1 += 100;
        ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
        delay(100);
      }
    }
    if ( abs(Ps3.data.analog.button.l1) > offset_val_shoulder) {
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
    motor->stepper_Y->setSpeed(motor->mspeed3);
    motor->stepper_Z->setSpeed(motor->mspeed2);

    if (motor->mspeed1 || motor->mspeed3 || motor->mspeed2) {
      motor->isforever = true;
    }
    else {
      motor->isforever = false;
    }
  }

}
#endif