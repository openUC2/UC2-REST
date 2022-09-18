#include "LaserController.h"

LaserController::LaserController(/* args */)
{
}

LaserController::~LaserController()
{
}



void LaserController::LASER_despeckle(int LASERdespeckle, int LASERid, int LASERperiod) {

  if (!isBusy) {

    int LASER_val_wiggle = 0;
    int PWM_CHANNEL_LASER = 0;
    if (LASERid == 1) {
      LASER_val_wiggle = LASER_val_1;
      PWM_CHANNEL_LASER = PWM_CHANNEL_LASER_1;
    }
    else if (LASERid == 2) {
      LASER_val_wiggle = LASER_val_2;
      PWM_CHANNEL_LASER = PWM_CHANNEL_LASER_2;
    }
    else if (LASERid == 3) {
      LASER_val_wiggle = LASER_val_3;
      PWM_CHANNEL_LASER = PWM_CHANNEL_LASER_3;
    }
    // add random number to current value to let it oscliate
    long laserwiggle = random(-LASERdespeckle, LASERdespeckle);
    LASER_val_wiggle += laserwiggle;
    if (LASER_val_wiggle > pwm_max)
      LASER_val_wiggle -= (2 * abs(laserwiggle));
    if (LASER_val_wiggle < 0)
      LASER_val_wiggle += (2 * abs(laserwiggle));

    if (DEBUG) Serial.println(LASERid);
    if (DEBUG) Serial.println(LASER_val_wiggle);

    ledcWrite(PWM_CHANNEL_LASER, LASER_val_wiggle);

    delay(LASERperiod);

  }
}


// Custom function accessible by the API
void LaserController::act() {
  // here you can do something
  Serial.println("LASER_act_fct");

  isBusy = true;

  int LASERid = (*WifiController::getJDoc())["LASERid"];
  int LASERval = (*WifiController::getJDoc())["LASERval"];
  int LASERdespeckle = (*WifiController::getJDoc())["LASERdespeckle"];
  int LASERdespecklePeriod = 20;
  if (WifiController::getJDoc()->containsKey("LASERdespecklePeriod")) {
    LASERdespecklePeriod = (*WifiController::getJDoc())["LASERdespecklePeriod"];
  }

  if (DEBUG) {
    Serial.print("LASERid "); Serial.println(LASERid);
    Serial.print("LASERval "); Serial.println(LASERval);
    Serial.print("LASERdespeckle "); Serial.println(LASERdespeckle);
    Serial.print("LASERdespecklePeriod "); Serial.println(LASERdespecklePeriod);
  }

  if (LASERid == 1) {
    LASER_val_1 = LASERval;
    LASER_despeckle_1 = LASERdespeckle;
    LASER_despeckle_period_1 = LASERdespecklePeriod;
    if (DEBUG) {
      Serial.print("LaserPIN ");
      Serial.println(pins->LASER_PIN_1);
    }
    ledcWrite(PWM_CHANNEL_LASER_1, LASERval);
  }
  else if (LASERid == 2) {
    LASER_val_2 = LASERval;
    LASER_despeckle_2 = LASERdespeckle;
    LASER_despeckle_period_2 = LASERdespecklePeriod;
    if (DEBUG) {
      Serial.print("LaserPIN ");
      Serial.println(pins->LASER_PIN_2);
    }
    ledcWrite(PWM_CHANNEL_LASER_2, LASERval);
  }
  else if (LASERid == 3) {
    LASER_val_3 = LASERval;
    LASER_despeckle_3 = LASERdespeckle;
    LASER_despeckle_period_3 = LASERdespecklePeriod;
    if (DEBUG) {
      Serial.print("LaserPIN ");
      Serial.println(pins->LASER_PIN_3);
    }
    ledcWrite(PWM_CHANNEL_LASER_3, LASERval);
  }

  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["return"] = 1;

  isBusy = false;
}

void LaserController::set() {
  // here you can set parameters
  int LASERid = (*WifiController::getJDoc())["LASERid"];
  int LASERpin = (*WifiController::getJDoc())["LASERpin"];

  if (DEBUG) Serial.print("LASERid "); Serial.println(LASERid);
  if (DEBUG) Serial.print("LASERpin "); Serial.println(LASERpin);

  if (LASERid != NULL and LASERpin != NULL) {
    if (LASERid == 1) {
      pins->LASER_PIN_1 = LASERpin;
      pinMode(pins->LASER_PIN_1, OUTPUT);
      digitalWrite(pins->LASER_PIN_1, LOW);
      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_LASER_1, pwm_frequency, pwm_resolution);
      ledcAttachPin(pins->LASER_PIN_1, PWM_CHANNEL_LASER_1);
      ledcWrite(PWM_CHANNEL_LASER_1, 0);
    }
    else if (LASERid == 2) {
      pins->LASER_PIN_2 = LASERpin;
      pinMode(pins->LASER_PIN_2, OUTPUT);
      digitalWrite(pins->LASER_PIN_2, LOW);
      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_LASER_2, pwm_frequency, pwm_resolution);
      ledcAttachPin(pins->LASER_PIN_2, PWM_CHANNEL_LASER_2);
      ledcWrite(PWM_CHANNEL_LASER_2, 0);
    }
    else if (LASERid == 3) {
      pins->LASER_PIN_3 = LASERpin;
      pinMode(pins->LASER_PIN_3, OUTPUT);
      digitalWrite(pins->LASER_PIN_3, LOW);
      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_LASER_3, pwm_frequency, pwm_resolution);
      ledcAttachPin(pins->LASER_PIN_3, PWM_CHANNEL_LASER_3);
      ledcWrite(PWM_CHANNEL_LASER_3, 0);
    }
  }

  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["return"] = 1;
}

// Custom function accessible by the API
void LaserController::get() {
  // GET SOME PARAMETERS HERE
  int LASERid = (*WifiController::getJDoc())["LASERid"];
  int LASERpin = 0;
  int LASERval = 0;

  if (LASERid == 1) {
    if (DEBUG) Serial.println("LASER 1");
    LASERpin = pins->LASER_PIN_1;
    LASERval = LASER_val_1;
  }
  else if (LASERid == 2) {
    if (DEBUG) Serial.println("AXIS 2");
    if (DEBUG) Serial.println("LASER 2");
    LASERpin = pins->LASER_PIN_2;
    LASERval = LASER_val_2;
  }
  else if (LASERid == 3) {
    if (DEBUG) Serial.println("AXIS 3");
    if (DEBUG) Serial.println("LASER 1");
    LASERpin = pins->LASER_PIN_3;
    LASERval = LASER_val_3;
  }

  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["LASERid"] = LASERid;
  (*WifiController::getJDoc())["LASERval"] = LASERval;
  (*WifiController::getJDoc())["LASERpin"] = LASERpin;
}

void LaserController::setup(PINDEF * pins) {
  Serial.println("Setting Up LASERs");
  this->pins = pins;
  // switch of the LASER directly
  pinMode(pins->LASER_PIN_1, OUTPUT);
  pinMode(pins->LASER_PIN_2, OUTPUT);
  pinMode(pins->LASER_PIN_3, OUTPUT);
  digitalWrite(pins->LASER_PIN_1, LOW);
  digitalWrite(pins->LASER_PIN_2, LOW);
  digitalWrite(pins->LASER_PIN_3, LOW);

  /* setup the PWM ports and reset them to 0*/
  ledcSetup(PWM_CHANNEL_LASER_1, pwm_frequency, pwm_resolution);
  ledcAttachPin(pins->LASER_PIN_1, PWM_CHANNEL_LASER_1);
  ledcWrite(PWM_CHANNEL_LASER_1, 10000);
  delay(500);
  ledcWrite(PWM_CHANNEL_LASER_1, 0);

  ledcSetup(PWM_CHANNEL_LASER_2, pwm_frequency, pwm_resolution);
  ledcAttachPin(pins->LASER_PIN_2, PWM_CHANNEL_LASER_2);
  ledcWrite(PWM_CHANNEL_LASER_2, 10000);
  delay(500);
  ledcWrite(PWM_CHANNEL_LASER_2, 0);

  ledcSetup(PWM_CHANNEL_LASER_3, pwm_frequency, pwm_resolution);
  ledcAttachPin(pins->LASER_PIN_3, PWM_CHANNEL_LASER_3);
  ledcWrite(PWM_CHANNEL_LASER_3, 10000);
  delay(500);
  ledcWrite(PWM_CHANNEL_LASER_3, 0);
}

void LaserController::loop()
{
  // attempting to despeckle by wiggeling the temperature-dependent modes of the laser?
  if (LASER_despeckle_1 > 0 && LASER_val_1 > 0)
    LASER_despeckle(LASER_despeckle_1, 1, LASER_despeckle_period_1);
  if (LASER_despeckle_2 > 0 && LASER_val_2 > 0)
    LASER_despeckle(LASER_despeckle_2, 2, LASER_despeckle_period_2);
  if (LASER_despeckle_3 > 0 && LASER_val_3 > 0)
    LASER_despeckle(laser.LASER_despeckle_3, 3, LASER_despeckle_period_3);
}
