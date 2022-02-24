#ifdef IS_PS4
#include <PS4Controller.h>

int offset_val = 30;
int stick_ly = 0;
int stick_lx = 0;
int stick_rx = 0;
int stick_ry = 0;

boolean stepper_1_on = false;
boolean stepper_2_on = false;
boolean stepper_3_on = false;

boolean stepper_1_running = false;
boolean stepper_2_running = false;
boolean stepper_3_running = false;

static inline int8_t sgn(int val) {
  if (val < 0) return -1;
  if (val == 0) return 0;
  return 1;
}

void control_PS4() {
  if (PS4.isConnected()) {

    /*
       ANALOG LEFT
    */
    if ( abs(PS4.LStickY()) > offset_val) {
      // move_z
      stick_ly = PS4.LStickY();
      stick_ly = stick_ly - sgn(stick_ly) * offset_val;
      run_motor(sgn(stick_ly) * 5, stick_ly * 5, 1);
      stepper_1_on = true;
      if (DEBUG) if(DEBUG) Serial.println("Motor 1: " + String(stick_ly));

    }
    else {
      if (stepper_1_on) {
        stepper_1_on = false;
        stick_ly = 0;
        run_motor(0, 0, 1); // switch motor off;
        digitalWrite(ENABLE, HIGH);
      }
    }

    /*
       ANALOG right
    */
    if ( (abs(PS4.RStickX()) > offset_val) and not stepper_3_running) {
      // move_x
      stepper_2_running = true;
      stick_rx = PS4.RStickX();
      stick_rx = stick_rx - sgn(stick_rx) * offset_val;
      run_motor(sgn(stick_rx) * 5, stick_rx * 5, 2);
      stepper_2_on = true;
      if (DEBUG) if(DEBUG) Serial.println("Motor 2: " + String(stick_rx));
    }
    else {
      if (stepper_2_on) {
        stepper_2_on = false;
        stick_rx = 0;
        stepper_2_running = false;
        run_motor(0, 0, 2); // switch motor off;
        digitalWrite(ENABLE, HIGH);
      }
    }

    if ( (abs(PS4.RStickY()) > offset_val) and not stepper_2_running) {
      // move_y
      stick_ry = PS4.RStickY();
      stepper_3_running = true;
      stick_ry = stick_ry - sgn(stick_ry) * offset_val;
      run_motor(sgn(stick_ry) * 5, stick_ry * 5, 3);
      stepper_3_on = true;
      if (DEBUG) if(DEBUG) Serial.println("Motor 3: " + String(stick_ry));
    }
    else {
      if (stepper_3_on) {
        stepper_3_on = false;
        stick_ly = 0;
        stepper_3_running = false;
        run_motor(0, 0, 3); // switch motor off;
        digitalWrite(ENABLE, HIGH);
      }
    }


    if (PS4.Down()) {
      // fine focus +
      run_motor(10,10,3);
      delay(100);
      run_motor(0,0,3);
    }
    if ( PS4.Up()) {
      // fine focus -
      run_motor(-10,-10,3);
      delay(100);
run_motor(0,0,3);
}


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
        /*
           Keypad left
        */
        if ( PS4.Left()) {
          // fine lens -
          analogout_val_1 -= 1;
          delay(100);
          ledcWrite(PWM_CHANNEL_analogout_1, analogout_val_1);
        }
        if ( PS4.Right()) {
          // fine lens +
          analogout_val_1 += 1;
          delay(100);
          ledcWrite(PWM_CHANNEL_analogout_1, analogout_val_1);
        }
        if ( PS4.Share()) {
          // reset
          analogout_val_1 = 0;
          ledcWrite(PWM_CHANNEL_analogout_1, analogout_val_1);
        }
    
        int offset_val_shoulder = 5;
        if ( PS4.R2()) {
          // analogout_val_1++ coarse
          if ((analogout_val_1 + 1000 < pwm_max)) {
            analogout_val_1 += 1000;
            ledcWrite(PWM_CHANNEL_analogout_1, analogout_val_1);
          }
          if(DEBUG) Serial.println(analogout_val_1);
          delay(100);
        }
    
        if ( PS4.L2()) {
          // analogout_val_1-- coarse
          if ((analogout_val_1 - 1000 > 0)) {
            analogout_val_1 -= 1000;
          ledcWrite(PWM_CHANNEL_analogout_1, analogout_val_1);
          }
          if(DEBUG) Serial.println(analogout_val_1);
          delay(100);
        }
    
    
        if (PS4.R1()) {
          // analogout_val_1 + semi coarse
          if ((analogout_val_1 + 100 < pwm_max)) {
            analogout_val_1 += 100;
            ledcWrite(PWM_CHANNEL_analogout_1, analogout_val_1);
            delay(100);
          }
        }
        if ( PS4.L1()) {
          // analogout_val_1 - semi coarse
          if ((analogout_val_1 - 100 > 0)) {
            analogout_val_1 -= 100;
            ledcWrite(PWM_CHANNEL_analogout_1, analogout_val_1);
            delay(50);
          }
        }
    
    
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

  }
}




void run_motor(int steps, int speed, int axis) {
  if (axis == 1) {
    stepper_X.begin(abs(speed));
    stepper_X.rotate(steps);
  }
  else if (axis == 2) {
    stepper_Y.begin(abs(speed));
    stepper_Y.rotate(steps);
  }
  else if (axis == 3) {
    stepper_Z.begin(abs(speed));
    stepper_Z.rotate(steps);
  }
}






#endif
