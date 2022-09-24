#include "ps_3_4_controller.h"

ps_3_4_controller::ps_3_4_controller()
{
    //ps_c = this;
}

ps_3_4_controller::~ps_3_4_controller()
{
    //ps_c = nullptr;
}

int ps_3_4_controller::is_cross()
{
    #ifdef IS_PS4
      return PS4.event.button_down.cross;
    #endif
    #ifdef IS_PS3
      return Ps3.event.button_down.cross;
    #endif
    
}

int ps_3_4_controller::is_share()
{
    #ifdef IS_PS4
      return PS4.event.button_down.share;
    #elif IS_PS3
      return Ps3.event.button_down.share;
    #else
      return 0;
    #endif
}

int ps_3_4_controller::is_square()
{
    #ifdef IS_PS4
      return PS4.event.button_down.square;
    #endif
    #ifdef IS_PS3
      return Ps3.event.button_down.square;
    #endif
}

int ps_3_4_controller::is_circle()
{
    #ifdef IS_PS4
      return PS4.event.button_down.circle;
    #endif
    #ifdef IS_PS3
      return Ps3.event.button_down.circle;
    #endif
}

int ps_3_4_controller::is_triangle()
{
    #ifdef IS_PS4
      return PS4.event.button_down.triangle;
    #endif
    #ifdef IS_PS3
      return Ps3.event.button_down.triangle;
    #endif
}

int ps_3_4_controller::is_down_down()
{
    #ifdef IS_PS4
      return PS4.event.button_down.down;
    #endif
    #ifdef IS_PS3
      return Ps3.event.button_down.down;
    #endif
}

int ps_3_4_controller::is_down_up()
{
    #ifdef IS_PS4
      return PS4.event.button_down.up;
    #endif
    #ifdef IS_PS3
      return Ps3.event.button_down.up;
    #endif
}

int ps_3_4_controller::is_down_left()
{
    #ifdef IS_PS4
      return PS4.event.button_down.left;
    #endif
    #ifdef IS_PS3
      return Ps3.event.button_down.left;
    #endif
}

int ps_3_4_controller::is_down_right()
{
    #ifdef IS_PS4
      return PS4.event.button_down.right;
    #endif
    #ifdef IS_PS3
      return Ps3.event.button_down.right;
    #endif
}

int ps_3_4_controller::is_charging()
{
    #ifdef IS_PS4
      return PS4.Charging();
    #endif
    #ifdef IS_PS3
      return Ps3.Charging();
    #endif
}

int ps_3_4_controller::is_connected()
{
    #ifdef IS_PS4
      return PS4.isConnected();
    #endif
    #ifdef IS_PS3
      return Ps3.isConnected();
    #endif
}

int ps_3_4_controller::analog_ly()
{
    #ifdef IS_PS4
      return PS4.data.analog.stick.ly;
    #endif
    #ifdef IS_PS3
      return Ps3.data.analog.stick.ly;
    #endif
}

int ps_3_4_controller::analog_lx()
{
    #ifdef IS_PS4
      return PS4.data.analog.stick.lx;
    #endif
    #ifdef IS_PS3
      return Ps3.data.analog.stick.lx;
    #endif
}

int ps_3_4_controller::analog_ry()
{
    #ifdef IS_PS4
      return PS4.data.analog.stick.ry;
    #endif
    #ifdef IS_PS3
      return Ps3.data.analog.stick.ry;
    #endif
}

int ps_3_4_controller::analog_rx()
{
    #ifdef IS_PS4
      return PS4.data.analog.stick.rx;
    #endif
    #ifdef IS_PS3
      return Ps3.data.analog.stick.rx;
    #endif
}

int ps_3_4_controller::left()
{
    #ifdef IS_PS4
      return PS4.data.button.left;
    #endif
    #ifdef IS_PS3
      return Ps3.data.button.left;
    #endif
}

int ps_3_4_controller::right()
{
    #ifdef IS_PS4
      return PS4.data.button.right;
    #endif
    #ifdef IS_PS3
      return Ps3.data.button.right;
    #endif
}

/*int ps_3_4_controller::start_b()
{
    #ifdef IS_PS4
      return PS4.data.button.start;
    #endif
    #ifdef IS_PS3
      return Ps3.data.button.start;
    #endif
}*/

int ps_3_4_controller::r2()
{
    #ifdef IS_PS4
      return PS4.data.button.r2;
    #endif
    #ifdef IS_PS3
      return Ps3.data.button.r2;
    #endif
}

int ps_3_4_controller::l2()
{
    #ifdef IS_PS4
      return PS4.data.button.l2;
    #endif
    #ifdef IS_PS3
      return Ps3.data.button.l2;
    #endif
}

int ps_3_4_controller::r1()
{
    #ifdef IS_PS4
      return PS4.data.button.r1;
    #endif
    #ifdef IS_PS3
      return Ps3.data.button.r1;
    #endif
}

int ps_3_4_controller::l1()
{
    #ifdef IS_PS4
      return PS4.data.button.l1;
    #endif
    #ifdef IS_PS3
      return Ps3.data.button.l1;
    #endif
}


void ps_3_4_controller::start()
{
    Serial.println("Connnecting to the PS4 controller, please please press the magic round button in the center..");
    #ifdef IS_PS4
      PS4.attach(ps_onAttach,this);
      PS4.begin("1a:2b:3c:01:01:01 - UNICAST!");
      PS4.attachOnConnect(ps_onConnect,this);
      PS4.attachOnDisconnect(ps_onDisConnect,this);
    #endif
    #ifdef IS_PS3
      Ps3.attach(ps_onAttach,this);
      Ps3.begin("1a:2b:3c:01:01:01 - UNICAST!");
      Ps3.attachOnConnect(ps_onConnect,this);
      Ps3.attachOnDisconnect(ps_onDisConnect,this);
    #endif
    const char*  PS4_MACADDESS = "1a:2b:3c:01:01:01";
    Serial.println(PS4_MACADDESS);
    Serial.println("PS_3/4 controler is set up.");
}

void ps_3_4_controller::onConnect() {
  if (DEBUG) Serial.println("PS4 Controller Connected.");
  IS_PSCONTROLER_ACTIVE = true;
#ifdef IS_MOTOR
  motor.setEnableMotor(true);
#endif

  if (is_charging())
      Serial.println("The controller is charging");
  #ifdef IS_PS4
    Serial.printf("Battery Level : %d\n", PS4.Battery());
  #endif
  #ifdef IS_PS3
    Serial.printf("Battery Level : %d\n", Ps3.Battery());
  #endif
    Serial.println();
}

void ps_3_4_controller::onAttach() {
  #ifdef IS_PS4
    PS4.attach(ps_activate,this);
  #endif
  #ifdef IS_PS3
    Ps3.attach(ps_activate,this);
  #endif
}



void ps_3_4_controller::onDisConnect() {
  if (DEBUG) Serial.println("PS Controller Connected.");
#ifdef IS_MOTOR
  motor.setEnableMotor(false);
#endif
}



void ps_3_4_controller::activate() {
  // callback for events
  if(is_share()) {
    IS_PSCONTROLER_ACTIVE = !IS_PSCONTROLER_ACTIVE;
    if (DEBUG) Serial.print("Setting manual mode to: ");
    if (DEBUG) Serial.println(IS_PSCONTROLER_ACTIVE);
#ifdef IS_MOTOR
    motor.setEnableMotor(IS_PSCONTROLER_ACTIVE);
#endif
    delay(1000); //Debounce?
  }
#ifdef IS_LED
  if(is_cross()) {
    IS_PS_CONTROLER_LEDARRAY = !IS_PS_CONTROLER_LEDARRAY;
    if (DEBUG) Serial.print("Turning LED Matrix to (cross): ");
    if (DEBUG) Serial.println(IS_PS_CONTROLER_LEDARRAY);
    LedController::set_all(255*IS_PS_CONTROLER_LEDARRAY,255*IS_PS_CONTROLER_LEDARRAY,255*IS_PS_CONTROLER_LEDARRAY);
    delay(1000); //Debounce?
  }
  if (is_circle()) {
    IS_PS_CONTROLER_LEDARRAY = !IS_PS_CONTROLER_LEDARRAY;
    if (DEBUG) Serial.print("Turning LED Matrix to (circle): ");
    if (DEBUG) Serial.println(IS_PS_CONTROLER_LEDARRAY);
    LedController::set_center(255*IS_PS_CONTROLER_LEDARRAY,255*IS_PS_CONTROLER_LEDARRAY,255*IS_PS_CONTROLER_LEDARRAY);
    delay(1000); //Debounce?

  }
#endif
  // LASER
#ifdef IS_LASER
  if (is_triangle()) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(laser.PWM_CHANNEL_LASER_1, 10000);
    delay(100); //Debounce?
  }
  if (is_square()) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(laser.PWM_CHANNEL_LASER_1, 0);
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
  if (is_down_up()) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(laser.PWM_CHANNEL_LASER_2, 20000);
    delay(100); //Debounce?
  }
  if (is_down_down()) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(laser.PWM_CHANNEL_LASER_2, 0);
    delay(100); //Debounce?
  }

  // LASER 2
  if (is_down_right()) {
    if (DEBUG) Serial.print("Turning on LAser 10000");
    ledcWrite(laser.PWM_CHANNEL_LASER_1, 20000);
    delay(100); //Debounce?
  }
  if (is_down_left()) {
    if (DEBUG) Serial.print("Turning off LAser ");
    ledcWrite(laser.PWM_CHANNEL_LASER_1, 0);
    delay(100); //Debounce?
  }
#endif
}



void ps_3_4_controller::control() {
  if (is_connected() && IS_PSCONTROLER_ACTIVE) {
#ifdef IS_MOTOR
      // Y-Direction
      if (abs(analog_ly()) > offset_val) {
        // move_z
        stick_ly = analog_ly();
        stick_ly = stick_ly - sgn(stick_ly) * offset_val;
        motor.mspeed2 =  stick_ly * 5 * global_speed;
        if (!motor.getEnableMotor())
          motor.setEnableMotor(true);
      }
      else if (motor.mspeed2 != 0) {
        motor.mspeed2 = 0;
        motor.stepper_Y->setSpeed(motor.mspeed2); // set motor off only once to not affect other modes
      }

      // Z-Direction
      if ((abs(analog_rx()) > offset_val)) {
        // move_x
        stick_rx = analog_rx();
        stick_rx = stick_rx - sgn(stick_rx) * offset_val;
        motor.mspeed3  = stick_rx * 5 * global_speed;
        if (motor.getEnableMotor())
          motor.setEnableMotor(true);
      }
      else if (motor.mspeed3 != 0) {
        motor.mspeed3 = 0;
        motor.stepper_Z->setSpeed(motor.mspeed3); // set motor off only once to not affect other modes
      }

      // X-direction
      if ((abs(analog_ry()) > offset_val)) {
        // move_y
        stick_ry = analog_ry();
        stick_ry = stick_ry - sgn(stick_ry) * offset_val;
        motor.mspeed1 = stick_ry * 5 * global_speed;
        if (!motor.getEnableMotor())
          motor.setEnableMotor(true);
      }
      else if (motor.mspeed1 != 0) {
        motor.mspeed1 = 0;
        motor.stepper_X->setSpeed(motor.mspeed1); // set motor off only once to not affect other modes
      }
#endif
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
      if (left()) {
        // fine lens -
        analog_val_1 -= 1;
        delay(100);
        ledcWrite(analog.PWM_CHANNEL_analog_1, analog_val_1);
      }
      if (right()) {
        // fine lens +
        analog_val_1 += 1;
        delay(100);
        ledcWrite(analog.PWM_CHANNEL_analog_1, analog_val_1);
      }
      //unknown button
      /*if (PS4.data.button.start) {
        // reset
        analog_val_1 = 0;
        ledcWrite(analog->PWM_CHANNEL_analog_1, analog_val_1);
      }*/

      int offset_val_shoulder = 5;
      if ( abs(r2()) > offset_val_shoulder) {
        // analog_val_1++ coarse
        if ((analog_val_1 + 1000 < pwm_max)) {
          analog_val_1 += 1000;
          ledcWrite(analog.PWM_CHANNEL_analog_1, analog_val_1);
        }
        if (DEBUG) Serial.println(analog_val_1);
        delay(100);
      }

      if ( abs(l2()) > offset_val_shoulder) {
        // analog_val_1-- coarse
        if ((analog_val_1 - 1000 > 0)) {
          analog_val_1 -= 1000;
          ledcWrite(analog.PWM_CHANNEL_analog_1, analog_val_1);
        }
        if (DEBUG) Serial.println(analog_val_1);
        delay(100);
      }


      if ( abs(r1()) > offset_val_shoulder) {
        // analog_val_1 + semi coarse
        if ((analog_val_1 + 100 < pwm_max)) {
          analog_val_1 += 100;
          ledcWrite(analog.PWM_CHANNEL_analog_1, analog_val_1);
          delay(100);
        }
      }
      if ( abs(l1()) > offset_val_shoulder) {
        // analog_val_1 - semi coarse
        if ((analog_val_1 - 100 > 0)) {
          analog_val_1 -= 100;
          ledcWrite(analog.PWM_CHANNEL_analog_1, analog_val_1);
          delay(50);
        }
      }

#endif

#ifdef IS_MOTOR
      // run all motors simultaneously
      motor.stepper_X->setSpeed(motor.mspeed1);
      motor.stepper_Y->setSpeed(motor.mspeed2);
      motor.stepper_Z->setSpeed(motor.mspeed3);

      if (motor.mspeed1 || motor.mspeed2 || motor.mspeed3) {
        motor.isforever = true;
      }
      else {
        motor.isforever = false;
      }
#endif
    }

}

void ps_3_4_controller::ps_onAttach(void * parameter)
{   
    ps_3_4_controller * psx = (ps_3_4_controller*)parameter;
    psx->onAttach();
};

void ps_3_4_controller::ps_onConnect(void * parameter){
    ps_3_4_controller * psx = (ps_3_4_controller*)parameter;
    psx->onConnect();
};
void ps_3_4_controller::ps_onDisConnect(void * parameter){
    ps_3_4_controller * psx = (ps_3_4_controller*)parameter;
    psx->onDisConnect();
};
void ps_3_4_controller::ps_activate(void * parameter){
    ps_3_4_controller * psx = (ps_3_4_controller*)parameter;
    psx->activate();
};

