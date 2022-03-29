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

int speed_x = 0;
int speed_y = 0;
int speed_z = 0;

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

bool control_PS3(bool override_overheating) {
  if (Ps3.isConnected()) {

    // Y-Direction
    if ( abs(Ps3.data.analog.stick.ly) > offset_val) {
      // move_z
      stick_ly = Ps3.data.analog.stick.ly;
      stick_ly = stick_ly - sgn(stick_ly) * offset_val;
      speed_y =  stick_ly * 5;
      stepper_1_on = true;
      override_overheating = false;
    }
    else {
      if (stepper_1_on) {
        stepper_1_on = false;
        stick_ly = 0;
        speed_y = 0;
      }
    }

    // Z-Direction
    if ( (abs(Ps3.data.analog.stick.rx) > offset_val)) {
      // move_x
      stepper_2_running = true;
      stick_rx = Ps3.data.analog.stick.rx;
      stick_rx = stick_rx - sgn(stick_rx) * offset_val;
      speed_z = stick_rx * 5;
      stepper_2_on = true;
      override_overheating = false;
    }
    else {
      if (stepper_2_on) {
        stepper_2_on = false;
        stick_rx = 0;
        stepper_2_running = false;
        speed_z = 0; // switch motor off;
      }
    }

    // X-direction
    if ( (abs(Ps3.data.analog.stick.ry) > offset_val)) {
      // move_y
      stick_ry = Ps3.data.analog.stick.ry;
      stepper_3_running = true;
      stick_ry = stick_ry - sgn(stick_ry) * offset_val;
      speed_x = stick_ry * 5;
      stepper_3_on = true;
      override_overheating = false;
    }
    else {
      if (stepper_3_on) {
        stepper_3_on = false;
        stick_ly = 0;
        stepper_3_running = false;
        speed_x = 0; // switch motor off;
      }
    }

    // fine control for Z using buttons
    if ( Ps3.data.button.down) {
      // fine focus +
      run_motor(0,0,10);
      delay(100);
    }
    if ( Ps3.data.button.up) {
      // fine focus -
      run_motor(0,0,-10);
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
    run_motor(speed_x, speed_y, speed_z);

    ps3_timeout_inactive = millis();;

    if (!override_overheating and ((ps3_timeout_inactive - ps3_timeout_active) > ps3_timeout)) {
      // prevent overheating
      ps3_is_enabled = false;
      digitalWrite(ENABLE, HIGH);
    }
  }
  return override_overheating;
}




void run_motor(int speed1, int speed2, int speed3) {

   // move all motors simultaneously by one step
  if (not ps3_is_enabled) {
    ps3_is_enabled = true;
    digitalWrite(ENABLE, LOW);
  }
  stepper_X.setSpeed(speed1 * 10);
  stepper_X.runSpeed();

  stepper_Y.setSpeed(speed2 * 10);
  stepper_Y.runSpeed();

  stepper_Z.setSpeed(speed3 * 10);
  stepper_Z.runSpeed();

  ps3_timeout_active = millis();

  
}



/* unused for now
 *  
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
