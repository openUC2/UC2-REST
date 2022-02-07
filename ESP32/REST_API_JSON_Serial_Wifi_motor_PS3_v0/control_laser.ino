#ifdef IS_LASER


// Custom function accessible by the API
DynamicJsonDocument LASER_act_fct() {
  // here you can do something
  Serial.println("LASER_act_fct");

  int LASERid = jsonDocument["LASERid"];
  int LASERval = jsonDocument["LASERval"];

  if (DEBUG) {
    Serial.print("LASERid "); Serial.println(LASERid);
    Serial.print("LASERval "); Serial.println(LASERval);
  }

  if (LASERid == 1) {
    LASER_val_1 = LASERval;
#ifdef IS_ESP32
    ledcWrite(PWM_CHANNEL_LASER_1, LASERval);
#else
    analogWrite(PWM_CHANNEL_LASER_1, LASERval);
#endif
  }
  else if (LASERid == 2) {
    LASER_val_2 = LASERval;
#ifdef IS_ESP32
    ledcWrite(PWM_CHANNEL_LASER_2, LASERval);
#else
    analogWrite(PWM_CHANNEL_LASER_2, LASERval);
#endif
  }
  else if (LASERid == 3) {
    LASER_val_3 = LASERval;
#ifdef IS_ESP32
    ledcWrite(PWM_CHANNEL_LASER_3, LASERval);
#else
    analogWrite(PWM_CHANNEL_LASER_3, LASERval);
#endif
  }

  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void LASER_set_fct() {
  // here you can set parameters
  int LASERid = jsonDocument["LASERid"];
  int LASERpin = jsonDocument["LASERpin"];

  if (DEBUG) Serial.print("LASERid "); Serial.println(LASERid);
  if (DEBUG) Serial.print("LASERpin "); Serial.println(LASERpin);

  if (LASERid != NULL and LASERpin != NULL) {
    if (LASERid == 1) {
      LASER_PIN_1 = LASERpin;
      pinMode(LASER_PIN_1, OUTPUT);
      digitalWrite(LASER_PIN_1, LOW);
#ifdef IS_ESP32
      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_LASER_1, pwm_frequency, pwm_resolution);
      ledcAttachPin(LASER_PIN_1, PWM_CHANNEL_LASER_1);
      ledcWrite(PWM_CHANNEL_LASER_1, 0);
#endif
    }
    else if (LASERid == 2) {
      LASER_PIN_2 = LASERpin;
      pinMode(LASER_PIN_2, OUTPUT);
      digitalWrite(LASER_PIN_2, LOW);
#ifdef IS_ESP32
      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_LASER_2, pwm_frequency, pwm_resolution);
      ledcAttachPin(LASER_PIN_2, PWM_CHANNEL_LASER_2);
      ledcWrite(PWM_CHANNEL_LASER_2, 0);
#endif
    }
    else if (LASERid == 3) {
      LASER_PIN_3 = LASERpin;
      pinMode(LASER_PIN_3, OUTPUT);
      digitalWrite(LASER_PIN_3, LOW);
#ifdef IS_ESP32
      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_LASER_3, pwm_frequency, pwm_resolution);
      ledcAttachPin(LASER_PIN_3, PWM_CHANNEL_LASER_3);
      ledcWrite(PWM_CHANNEL_LASER_3, 0);
#endif
  }
}

jsonDocument.clear();
jsonDocument["return"] = 1;
}

// Custom function accessible by the API
void LASER_get_fct() {
  // GET SOME PARAMETERS HERE
  int LASERid = jsonDocument["LASERid"];
  int LASERpin = 0;
  int LASERval = 0;

  if (LASERid == 1) {
    if (DEBUG) Serial.println("LASER 1");
    LASERpin = LASER_PIN_1;
    LASERval = LASER_val_1;
  }
  else if (LASERid == 2) {
    if (DEBUG) Serial.println("AXIS 2");
    if (DEBUG) Serial.println("LASER 2");
    LASERpin = LASER_PIN_2;
    LASERval = LASER_val_2;
  }
  else if (LASERid == 3) {
    if (DEBUG) Serial.println("AXIS 3");
    if (DEBUG) Serial.println("LASER 1");
    LASERpin = LASER_PIN_3;
    LASERval = LASER_val_3;
  }

  jsonDocument.clear();
  jsonDocument["LASERid"] = LASERid;
  jsonDocument["LASERval"] = LASERval;
  jsonDocument["LASERpin"] = LASERpin;
}


/*
  wrapper for HTTP requests
*/
#ifdef IS_WIFI
void LASER_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  LASER_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void LASER_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  LASER_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void LASER_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  LASER_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
