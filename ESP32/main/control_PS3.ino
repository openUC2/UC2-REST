#ifdef IS_PS3
int offset_val = 5;
int stick_ly = 0;
int stick_lx = 0;
int stick_rx = 0;
int stick_ry = 0;

int speed_x = 0;
int speed_y = 0;
int speed_z = 0;

int speedfactor = 2;

void onConnect() {
  if (DEBUG) Serial.println("PS3 Controller Connected.");
  setEnableMotor(true);
}
void onDisConnect() {
  if (DEBUG) Serial.println("PS3 Controller Connected.");
  setEnableMotor(false);
}


void control_PS3() {
  if (Ps3.isConnected()) {

    // Y-Direction
    if ( abs(Ps3.data.analog.stick.ly) > offset_val) {
      // move_z
      stick_ly = Ps3.data.analog.stick.ly;
      stick_ly = stick_ly - sgn(stick_ly) * offset_val;
      speed_y =  stick_ly * 5;
      if (not getEnableMotor())
        setEnableMotor(true);
    }
    else if (speed_y != 0) {
        speed_y = 0;
        stepper_Y.setSpeed(speed_y); // set motor off only once to not affect other modes
    }

    // Z-Direction
    if ( (abs(Ps3.data.analog.stick.rx) > offset_val)) {
      // move_x
      stick_rx = Ps3.data.analog.stick.rx;
      stick_rx = stick_rx - sgn(stick_rx) * offset_val;
      speed_z = stick_rx * 5;
      if (not getEnableMotor())
        setEnableMotor(true);
    }
    else if (speed_z != 0) {
        speed_z = 0;
        stepper_Z.setSpeed(speed_z); // set motor off only once to not affect other modes
    }

    // X-direction
    if ( (abs(Ps3.data.analog.stick.ry) > offset_val)) {
      // move_y
      stick_ry = Ps3.data.analog.stick.ry;
      stick_ry = stick_ry - sgn(stick_ry) * offset_val;
      speed_x = stick_ry * 5;
      if (not getEnableMotor())
        setEnableMotor(true);
    }
    else if (speed_x != 0) {
        speed_x = 0;
        stepper_X.setSpeed(speed_x); // set motor off only once to not affect other modes
    }

    // fine control for Z using buttons
    if ( Ps3.data.button.down) {
      // fine focus +
      run_motor(0, 0, 10);
      delay(100);
    }
    if ( Ps3.data.button.up) {
      // fine focus -
      run_motor(0, 0, -10);
      delay(100);

    }


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
    if (speed_x or speed_y or speed_z) {
      // the special case of 0 speed is handled in the joysticks
      run_motor(speed_x, speed_y, speed_z);
    }
  }
}




void run_motor(int speed_x, int speed_y, int speed_z) {

  // move all motors simultaneously by one step
  stepper_X.setSpeed(speed_x * speedfactor);
  stepper_X.setMaxSpeed(speed_x * speedfactor);
  stepper_Y.setSpeed(speed_y * speedfactor);
  stepper_Y.setMaxSpeed(speed_y * speedfactor);
  stepper_Z.setSpeed(speed_z * speedfactor);
  stepper_Z.setMaxSpeed(speed_z * speedfactor);
  isforever = true;
  isblock = false;
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
