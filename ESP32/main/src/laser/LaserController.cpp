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
void LaserController::LASER_act_fct() {
  // here you can do something
  Serial.println("LASER_act_fct");

  isBusy = true;

  int LASERid = (*jsonDocument)["LASERid"];
  int LASERval = (*jsonDocument)["LASERval"];
  int LASERdespeckle = (*jsonDocument)["LASERdespeckle"];
  int LASERdespecklePeriod = 20;
  if (jsonDocument->containsKey("LASERdespecklePeriod")) {
    LASERdespecklePeriod = (*jsonDocument)["LASERdespecklePeriod"];
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

  jsonDocument->clear();
  (*jsonDocument)["return"] = 1;

  isBusy = false;
}

void LaserController::LASER_set_fct() {
  // here you can set parameters
  int LASERid = (*jsonDocument)["LASERid"];
  int LASERpin = (*jsonDocument)["LASERpin"];

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

  jsonDocument->clear();
  (*jsonDocument)["return"] = 1;
}

// Custom function accessible by the API
void LaserController::LASER_get_fct() {
  // GET SOME PARAMETERS HERE
  int LASERid = (*jsonDocument)["LASERid"];
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

  jsonDocument->clear();
  (*jsonDocument)["LASERid"] = LASERid;
  (*jsonDocument)["LASERval"] = LASERval;
  (*jsonDocument)["LASERpin"] = LASERpin;
}

void LaserController::setup_laser() {
  Serial.println("Setting Up LASERs");

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

/*
  wrapper for HTTP requests
*/
void LaserController::LASER_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson((*jsonDocument), body);
  LASER_act_fct();
  serializeJson((*jsonDocument), output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void LaserController::LASER_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson((*jsonDocument), body);
  LASER_get_fct();
  serializeJson((*jsonDocument), output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void LaserController::LASER_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson((*jsonDocument), body);
  LASER_set_fct();
  serializeJson((*jsonDocument), output);
  server.send(200, "application/json", output);
}