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


bool ps3_is_enabled = false;
int ps3_last_enabled_triggered = -1;
int ps3_timeout_active = 0;
int ps3_timeout_inactive = 0;
int ps3_timeout = 2000; // wait 5 seconds between actions before motors switch off again

void onConnect() {
  if (DEBUG) Serial.println("PS3 Controller Connected.");
  ps3_timeout_active = millis();
  ps3_timeout_inactive = millis();
  ps3_is_enabled = false;
  digitalWrite(ENABLE, HIGH);
}
void onDisConnect() {
  if (DEBUG) Serial.println("PS3 Controller Connected.");
  
  digitalWrite(ENABLE, HIGH);
  
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
    }
    else {
      if (stepper_1_on) {
        stepper_1_on = false;
        stick_ly = 0;
        run_motor(0, 0, 1); // switch motor off;
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
    }
    else {
      if (stepper_2_on) {
        stepper_2_on = false;
        stick_rx = 0;
        stepper_2_running = false;
        run_motor(0, 0, 2); // switch motor off;
        
      }
    }

    if ( (abs(Ps3.data.analog.stick.ry) > offset_val) and not stepper_2_running) {
      // move_y
      stick_ry = Ps3.data.analog.stick.ry;
      stepper_3_running = true;
      stick_ry = stick_ry - sgn(stick_ry) * offset_val;
      run_motor(sgn(stick_ry) * 5, stick_ry * 5, 3);
      stepper_3_on = true;
    }
    else {
      if (stepper_3_on) {
        stepper_3_on = false;
        stick_ly = 0;
        stepper_3_running = false;
        run_motor(0, 0, 3); // switch motor off;
        
      }
    }


    if ( Ps3.data.button.down) {
      // fine focus +
      run_motor(10, 10, 3);
      delay(100);
      run_motor(0, 0, 3);
    }
    if ( Ps3.data.button.up) {
      // fine focus -
      run_motor(-10, -10, 3);
      delay(100);
      run_motor(0, 0, 3);
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

    ps3_timeout_inactive = millis();; 

    if ((ps3_timeout_inactive-ps3_timeout_active)>ps3_timeout){
      // prevent overheating
      ps3_is_enabled = false;
      digitalWrite(ENABLE, HIGH);
    }
  }
}




void run_motor(int steps, int speed, int axis) {

  if (not ps3_is_enabled){
    ps3_is_enabled=true;
    digitalWrite(ENABLE, LOW);
  }
    speed = speed * 10;
//  if (DEBUG) Serial.println("Motor "+String(axis)+" , steps: " + String(steps) + ", speed:" + String(speed));

//    for (int istep = 0; istep < steps; istep++) {
    if (axis == 1) {
      stepper_X.setSpeed(speed);
      stepper_X.runSpeed();
    }
    else if (axis == 2) {
      stepper_Y.setSpeed(speed);
      stepper_Y.runSpeed();
    }
    else if (axis == 3) {
      stepper_Z.setSpeed(speed);
      stepper_Z.runSpeed();
    }
    ps3_timeout_active = millis(); 

}






#endif
