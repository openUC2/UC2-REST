#ifdef IS_PS3
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

void onConnect() {
  Serial.println("PS3 Controller Connected.");
}

void control_PS3() {
  if (Ps3.isConnected()) {

    /*
       ANALOG LEFT
    */
    if ( abs(Ps3.data.analog.stick.ly) > offset_val) {
      // move_z
      stick_ly = Ps3.data.analog.stick.ly;
      stick_ly = stick_ly - sgn(stick_ly) * offset_val;
      run_motor(sgn(stick_ly) * 5, stick_ly * 5, 1);
      stepper_1_on = true;
      if (DEBUG) Serial.println("Motor 1: " + String(stick_ly));

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
    if ( (abs(Ps3.data.analog.stick.rx) > offset_val) and not stepper_3_running) {
      // move_x
      stepper_2_running = true;
      stick_rx = Ps3.data.analog.stick.rx;
      stick_rx = stick_rx - sgn(stick_rx) * offset_val;
      run_motor(sgn(stick_rx) * 5, stick_rx * 5, 2);
      stepper_2_on = true;
      if (DEBUG) Serial.println("Motor 2: " + String(stick_rx));
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

    if ( (abs(Ps3.data.analog.stick.ry) > offset_val) and not stepper_2_running) {
      // move_y
      stick_ry = Ps3.data.analog.stick.ry;
      stepper_3_running = true;
      stick_ry = stick_ry - sgn(stick_ry) * offset_val;
      run_motor(sgn(stick_ry) * 5, stick_ry * 5, 3);
      stepper_3_on = true;
      if (DEBUG) Serial.println("Motor 3: " + String(stick_ry));
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


    if ( Ps3.data.button.down) {
      // fine focus +
      run_stepper_3(10, 10);
      delay(100);
      run_stepper_3(0, 0);
    }
    if ( Ps3.data.button.up) {
      // fine focus -
      run_stepper_3(-10, -10);
      delay(100);
      run_stepper_3(0, 0);
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
    //    /*
    //       Keypad left
    //    */
    //    if ( Ps3.data.button.left) {
    //      // fine lens -
    //      lens_x -= 1;
    //      delay(100);
    //      run_lens_x(lens_x);
    //    }
    //    if ( Ps3.data.button.right) {
    //      // fine lens +
    //      lens_x += 1;
    //      delay(100);
    //      run_lens_x(lens_x);
    //    }
    //    if ( Ps3.data.button.start) {
    //      // reset
    //      lens_z = 0;
    //      lens_x = 0;
    //      run_lens_z(lens_z);
    //      run_lens_x(lens_x);
    //      strip.setPixelColor(0, strip.Color(0, 0, 0));
    //      strip.show();
    //      is_laser_red = false;
    //      laserval = 0;
    //      run_laser(0);
    //    }
    //
    //    int offset_val_shoulder = 5;
    //    if ( abs(Ps3.data.analog.button.r2) > offset_val_shoulder) {
    //      // lens_x++ coarse
    //      if ((lens_x + 1000 < pwm_max)) {
    //        lens_x += 1000;
    //        run_lens_x(lens_x);
    //      }
    //      Serial.println(lens_x);
    //      delay(100);
    //    }
    //
    //    if ( abs(Ps3.data.analog.button.l2) > offset_val_shoulder) {
    //      // lens_x-- coarse
    //      if ((lens_x - 1000 > 0)) {
    //        lens_x -= 1000;
    //        run_lens_x(lens_x);
    //      }
    //      Serial.println(lens_x);
    //      delay(100);
    //    }
    //
    //
    //    if ( abs(Ps3.data.analog.button.r1) > offset_val_shoulder) {
    //      // lens_x + semi coarse
    //      if ((lens_x + 100 < pwm_max)) {
    //        lens_x += 100;
    //        run_lens_x(lens_x);
    //        delay(100);
    //      }
    //    }
    //    if ( abs(Ps3.data.analog.button.l1) > offset_val_shoulder) {
    //      // lens_x - semi coarse
    //      if ((lens_x - 100 > 0)) {
    //        lens_x -= 100;
    //        run_lens_x(lens_x);
    //        delay(50);
    //      }
    //    }
    //
    //
    //    if ( Ps3.data.button.circle ) {
    //      //if(not is_laser_red){
    //      Serial.println("Laser on");
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
    //        Serial.println("Laser off");
    //        is_laser_red = false;
    //        laserval = 0;
    //        run_laser(0);
    //      }
    //
    //    }
    //
    //    if ( Ps3.data.button.triangle) {
    //      if (not is_sofi) {
    //        Serial.println("SOFI on");
    //        is_sofi = true;
    //        glob_amplitude_sofi = 300;
    //      }
    //    }
    //
    //    if ( Ps3.data.button.square ) {
    //      if (is_sofi) {
    //        is_sofi = false;
    //        Serial.println("SOFI off");
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
