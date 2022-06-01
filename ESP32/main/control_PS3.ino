#ifdef IS_PS3
#include "parameters_ps3.h"

bool IS_PS3_CONTROLER_LEDARRAY = false;
void onConnect() {
  if (DEBUG) Serial.println("PS3 Controller Connected.");
  IS_PSCONTROLER_ACTIVE = true;
  setEnableMotor(true);
}

void onAttach() {
  Ps3.attach(activate_PS3);
}


void onDisConnect() {
  if (DEBUG) Serial.println("PS3 Controller Connected.");
  setEnableMotor(false);
}


void activate_PS3() {
  // callback for events
  if (Ps3.event.button_down.select) {
    IS_PSCONTROLER_ACTIVE = !IS_PSCONTROLER_ACTIVE;
    if (DEBUG) Serial.print("Setting manual mode to: ");
    if (DEBUG) Serial.println(IS_PSCONTROLER_ACTIVE);
    setEnableMotor(IS_PSCONTROLER_ACTIVE);
    delay(1000); //Debounce?
  }
  if (Ps3.event.button_down.cross) {
    IS_PS3_CONTROLER_LEDARRAY = !IS_PS3_CONTROLER_LEDARRAY;
    if (DEBUG) Serial.print("Turning LED Matrix to: ");
    if (DEBUG) Serial.println(IS_PS3_CONTROLER_LEDARRAY);
#ifdef IS_LEDARR
    set_all(255 * IS_PS3_CONTROLER_LEDARRAY, 255 * IS_PS3_CONTROLER_LEDARRAY, 255 * IS_PS3_CONTROLER_LEDARRAY);
#endif
    delay(1000); //Debounce?
  }


  // LASER 1
  if (Ps3.event.button_down.up) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(PWM_CHANNEL_LASER_2, 20000);
    delay(100); //Debounce?
  }
  if (Ps3.event.button_down.down) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(PWM_CHANNEL_LASER_2, 0);
    delay(100); //Debounce?
  }

  // LASER 2
  if (Ps3.event.button_down.right) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(PWM_CHANNEL_LASER_1, 20000);
    delay(100); //Debounce?
  }
  if (Ps3.event.button_down.left) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(PWM_CHANNEL_LASER_1, 0);
    delay(100); //Debounce?
  }


}

void control_PS3() {
  if (Ps3.isConnected() and IS_PSCONTROLER_ACTIVE) {
    // Y-Direction
    if ( abs(Ps3.data.analog.stick.ly) > offset_val) {
      // move_z
      stick_ly = Ps3.data.analog.stick.ly;
      stick_ly = stick_ly - sgn(stick_ly) * offset_val;
      if (abs(stick_ly) > 100)
        stick_ly *= 2;

      mspeed3 =  stick_ly * 5 * global_speed;
      if (not getEnableMotor())
        setEnableMotor(true);
    }
    else if (mspeed3 != 0) {
      mspeed3 = 0;
      stepper_Y.setSpeed(mspeed3); // set motor off only once to not affect other modes
    }

    // Z-Direction
    if ( (abs(Ps3.data.analog.stick.rx) > offset_val)) {
      // move_x
      stick_rx = Ps3.data.analog.stick.rx;
      stick_rx = stick_rx - sgn(stick_rx) * offset_val;

      if (abs(stick_rx) > 100)
        stick_rx *= 2;

      mspeed2  = stick_rx * 5 * global_speed;
      if (not getEnableMotor())
        setEnableMotor(true);
    }
    else if (mspeed2 != 0) {
      mspeed2 = 0;
      stepper_Z.setSpeed(mspeed2); // set motor off only once to not affect other modes
    }

    // X-direction
    if ( (abs(Ps3.data.analog.stick.ry) > offset_val)) {
      // move_y
      stick_ry = Ps3.data.analog.stick.ry;
      stick_ry = stick_ry - sgn(stick_ry) * offset_val;
      if (abs(stick_ry) > 100)
        stick_ry *= 2;
      mspeed1 = stick_ry * 5 * global_speed;
      if (not getEnableMotor())
        setEnableMotor(true);
    }
    else if (mspeed1 != 0) {
      mspeed1 = 0;
      stepper_X.setSpeed(mspeed1); // set motor off only once to not affect other modes
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
      ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
    }
    if ( Ps3.data.button.right) {
      // fine lens +
      analog_val_1 += 1;
      delay(100);
      ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
    }
    if ( Ps3.data.button.start) {
      // reset
      analog_val_1 = 0;
      ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
    }

    int offset_val_shoulder = 5;
    if ( abs(Ps3.data.analog.button.r2) > offset_val_shoulder) {
      // analog_val_1++ coarse
      if ((analog_val_1 + 1000 < pwm_max)) {
        analog_val_1 += 1000;
        ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
      }
      if (DEBUG) Serial.println(analog_val_1);
      delay(100);
    }

    if ( abs(Ps3.data.analog.button.l2) > offset_val_shoulder) {
      // analog_val_1-- coarse
      if ((analog_val_1 - 1000 > 0)) {
        analog_val_1 -= 1000;
        ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
      }
      if (DEBUG) Serial.println(analog_val_1);
      delay(100);
    }


    if ( abs(Ps3.data.analog.button.r1) > offset_val_shoulder) {
      // analog_val_1 + semi coarse
      if ((analog_val_1 + 100 < pwm_max)) {
        analog_val_1 += 100;
        ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
        delay(100);
      }
    }
    if ( abs(Ps3.data.analog.button.l1) > offset_val_shoulder) {
      // analog_val_1 - semi coarse
      if ((analog_val_1 - 100 > 0)) {
        analog_val_1 -= 100;
        ledcWrite(PWM_CHANNEL_analog_1, analog_val_1);
        delay(50);
      }
    }

#endif

    // run all motors simultaneously
    stepper_X.setSpeed(mspeed1);
    stepper_Y.setSpeed(mspeed3);
    stepper_Z.setSpeed(mspeed2);

    if (mspeed1 or mspeed3 or mspeed2) {
      isforever = true;
    }
    else {
      isforever = false;
    }
  }

}


/* unused for now

*/


//
//    if ( abs(Ps3.data.analog.stick.lx) > offset_val) {
//      // LED Stip
//      stick_lx = Ps3.data.analog.stick.lx;
//      //stick_lx = stick_lx + sgn(stick_lx) * offset_val;
//      if ((colour_led += sgn(stick_lx) * 5) > 0 and (colour_led += sgn(stick_lx) * 5) < 255)
//        colour_led += sgn(stick_lx) * 5;
//      if (colour_led < 0)
//        colour_led = 0;
//      strip.setPixelColor(0, strip.Color(colour_led, colour_led, colour_led));
//      strip.show();
//      delay(100);
//    }
//
//

//    if ( Ps3.data.button.circle ) {
//      //if(not is_laser_red){
//      if(DEBUG) Serial.println("Laser on");
//      is_laser_red = true;
//      laserval += 200;
//      run_laser(laserval);
//      delay(100);
//      //}
//
//    }
//
//    if ( Ps3.data.button.cross ) {
//      if (is_laser_red) {
//        if(DEBUG) Serial.println("Laser off");
//        is_laser_red = false;
//        laserval = 0;
//        run_laser(0);
//      }
//
//    }
//
//    if ( Ps3.data.button.triangle) {
//      if (not is_sofi) {
//        if(DEBUG) Serial.println("SOFI on");
//        is_sofi = true;
//        glob_amplitude_sofi = 300;
//      }
//    }
//
//    if ( Ps3.data.button.square ) {
//      if (is_sofi) {
//        is_sofi = false;
//        if(DEBUG) Serial.println("SOFI off");
//        glob_amplitude_sofi = 0;
//      }
//
//    }


#endif
