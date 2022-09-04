#include "ps3_controller.h" 
#include <Ps3Controller.h>
#include <functional>

Ps3Controller ps3Controller;

gamepad::gamepad()
{
    gp = this;
}

void gamepad::start()
{
    Serial.println("Connnecting to the PS3 controller, please please the magic round button in the center..");
    ps3Controller.attach(gamepad_onAttach);
    ps3Controller.attachOnConnect(gamepad_onConnect);
    ps3Controller.attachOnDisconnect(gamepad_onDisConnect);
    const char* PS3_MACADDESS = "01:02:03:04:05:06";
    ps3Controller.begin("01:02:03:04:05:06");
    Serial.println(PS3_MACADDESS);
    //String address = Ps3.getAddress(); // have arbitrary address?
    //Serial.println(address);
    Serial.println("PS3 controler is set up.");
}

void gamepad::onConnect()
{
    if (DEBUG) Serial.println("PS3 Controller Connected.");
    IS_PSCONTROLER_ACTIVE = true;
    focusmotor->setEnableMotor(true);
}

void gamepad::onAttach() {
  ps3Controller.attach(gamepad_activate);
}


void gamepad::onDisConnect() {
  if (DEBUG) Serial.println("PS3 Controller Connected.");
  focusmotor->setEnableMotor(false);
}


void gamepad::activate() {
  // callback for events
  if (ps3Controller.event.button_down.select) {
    IS_PSCONTROLER_ACTIVE = !IS_PSCONTROLER_ACTIVE;
    if (DEBUG) Serial.print("Setting manual mode to: ");
    if (DEBUG) Serial.println(IS_PSCONTROLER_ACTIVE);
    focusmotor->setEnableMotor(IS_PSCONTROLER_ACTIVE);
    delay(1000); //Debounce?
  }
  if (ps3Controller.event.button_down.cross) {
    IS_PS_CONTROLER_LEDARRAY = !IS_PS_CONTROLER_LEDARRAY;
    if (DEBUG) Serial.print("Turning LED Matrix to: ");
    if (DEBUG) Serial.println(IS_PS_CONTROLER_LEDARRAY);
    led->set_all(255 * IS_PS_CONTROLER_LEDARRAY, 255 * IS_PS_CONTROLER_LEDARRAY, 255 * IS_PS_CONTROLER_LEDARRAY);
    delay(1000); //Debounce?
  }


  // LASER 1
  if (ps3Controller.event.button_down.up) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(PWM_CHANNEL_LASER_2, 20000);
    delay(100); //Debounce?
  }
  if (ps3Controller.event.button_down.down) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(PWM_CHANNEL_LASER_2, 0);
    delay(100); //Debounce?
  }

  // LASER 2
  if (ps3Controller.event.button_down.right) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(PWM_CHANNEL_LASER_1, 20000);
    delay(100); //Debounce?
  }
  if (ps3Controller.event.button_down.left) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(PWM_CHANNEL_LASER_1, 0);
    delay(100); //Debounce?
  }


}

void gamepad::control() {
  if (ps3Controller.isConnected() and IS_PSCONTROLER_ACTIVE) {
    // Y-Direction
    if ( abs(ps3Controller.data.analog.stick.ly) > offset_val) {
      // move_z
      stick_ly = ps3Controller.data.analog.stick.ly;
      stick_ly = stick_ly - sgn(stick_ly) * offset_val;
      if (abs(stick_ly) > 100)
        stick_ly *= 2;

      focusmotor->mspeed3 =  stick_ly * 5 * global_speed;
      if (!focusmotor->getEnableMotor())
        focusmotor->setEnableMotor(true);
    }
    else if (focusmotor->mspeed3 != 0) {
      focusmotor->mspeed3 = 0;
      focusmotor->stepper_Y->setSpeed(focusmotor->mspeed3); // set motor off only once to not affect other modes
    }

    // Z-Direction
    if ( (abs(ps3Controller.data.analog.stick.rx) > offset_val)) {
      // move_x
      stick_rx = ps3Controller.data.analog.stick.rx;
      stick_rx = stick_rx - sgn(stick_rx) * offset_val;

      if (abs(stick_rx) > 100)
        stick_rx *= 2;

      focusmotor->mspeed2  = stick_rx * 5 * global_speed;
      if (!focusmotor->getEnableMotor())
        focusmotor->setEnableMotor(true);
    }
    else if (focusmotor->mspeed2 != 0) {
      focusmotor->mspeed2 = 0;
      focusmotor->stepper_Z->setSpeed(focusmotor->mspeed2); // set motor off only once to not affect other modes
    }

    // X-direction
    if ( (abs(ps3Controller.data.analog.stick.ry) > offset_val)) {
      // move_y
      stick_ry = ps3Controller.data.analog.stick.ry;
      stick_ry = stick_ry - sgn(stick_ry) * offset_val;
      if (abs(stick_ry) > 100)
        stick_ry *= 2;
      focusmotor->mspeed1 = stick_ry * 5 * global_speed;
      if (not focusmotor->getEnableMotor())
        focusmotor->setEnableMotor(true);
    }
    else if (focusmotor->mspeed1 != 0) {
      focusmotor->mspeed1 = 0;
      focusmotor->stepper_X->setSpeed(focusmotor->mspeed1); // set motor off only once to not affect other modes
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
    if ( ps3Controller.data.button.left) {
      // fine lens -
      analog_val_1 -= 1;
      delay(100);
      ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
    }
    if ( ps3Controller.data.button.right) {
      // fine lens +
      analog_val_1 += 1;
      delay(100);
      ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
    }
    if ( ps3Controller.data.button.start) {
      // reset
      analog_val_1 = 0;
      ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
    }

    int offset_val_shoulder = 5;
    if ( abs(ps3Controller.data.analog.button.r2) > offset_val_shoulder) {
      // analog_val_1++ coarse
      if ((analog_val_1 + 1000 < pwm_max)) {
        analog_val_1 += 1000;
        ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
      }
      if (DEBUG) Serial.println(analog_val_1);
      delay(100);
    }

    if ( abs(ps3Controller.data.analog.button.l2) > offset_val_shoulder) {
      // analog_val_1-- coarse
      if ((analog_val_1 - 1000 > 0)) {
        analog_val_1 -= 1000;
        ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
      }
      if (DEBUG) Serial.println(analog_val_1);
      delay(100);
    }


    if ( abs(ps3Controller.data.analog.button.r1) > offset_val_shoulder) {
      // analog_val_1 + semi coarse
      if ((analog_val_1 + 100 < pwm_max)) {
        analog_val_1 += 100;
        ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
        delay(100);
      }
    }
    if ( abs(ps3Controller.data.analog.button.l1) > offset_val_shoulder) {
      // analog_val_1 - semi coarse
      if ((analog_val_1 - 100 > 0)) {
        analog_val_1 -= 100;
        ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
        delay(50);
      }
    }

#endif

    // run all motors simultaneously
    focusmotor->stepper_X->setSpeed(focusmotor->mspeed1);
    focusmotor->stepper_Y->setSpeed(focusmotor->mspeed3);
    focusmotor->stepper_Z->setSpeed(focusmotor->mspeed2);

    if (focusmotor->mspeed1 or focusmotor->mspeed3 or focusmotor->mspeed2) {
      focusmotor->isforever = true;
    }
    else {
      focusmotor->isforever = false;
    }
  }

}