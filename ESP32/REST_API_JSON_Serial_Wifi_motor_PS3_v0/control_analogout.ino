#ifdef IS_ANALOGOUT

// Custom function accessible by the API
DynamicJsonDocument analogout_act_fct() {
  // here you can do something
  Serial.println("analogout_act_fct");

  int analogoutid = jsonDocument["analogoutid"];
  int analogoutval = jsonDocument["analogoutval"];

  if (DEBUG) {
    Serial.print("analogoutid "); Serial.println(analogoutid);
    Serial.print("analogoutval "); Serial.println(analogoutval);
  }

  if (analogoutid == 1) {
    analogout_val_1 = analogoutval;
    ledcWrite(PWM_CHANNEL_analogout_1, analogoutval);
  }
  else if (analogoutid == 2) {
    analogout_val_2 = analogoutval;
    ledcWrite(PWM_CHANNEL_analogout_2, analogoutval);
  }
  else if (analogoutid == 3) {
    analogout_val_3 = analogoutval;
    ledcWrite(PWM_CHANNEL_analogout_3, analogoutval);
  }
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void analogout_set_fct() {
  // here you can set parameters
  int analogoutid = jsonDocument["analogoutid"];
  int analogoutpin = jsonDocument["analogoutpin"];

  if (DEBUG) Serial.print("analogoutid "); Serial.println(analogoutid);
  if (DEBUG) Serial.print("analogoutpin "); Serial.println(analogoutpin);

  if (analogoutid != NULL and analogoutpin != NULL) {
    if (analogoutid == 1) {
      analogout_PIN_1 = analogoutpin;
      pinMode(analogout_PIN_1, OUTPUT);
      digitalWrite(analogout_PIN_1, LOW);
#ifdef IS_ESP32
      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_analogout_1, pwm_frequency, pwm_resolution);
      ledcAttachPin(analogout_PIN_1, PWM_CHANNEL_analogout_1);
      ledcWrite(PWM_CHANNEL_analogout_1, 0);
#endif
    }
    else if (analogoutid == 2) {
      analogout_PIN_2 = analogoutpin;
      pinMode(analogout_PIN_2, OUTPUT);
      digitalWrite(analogout_PIN_2, LOW);
#ifdef IS_ESP32
      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_analogout_2, pwm_frequency, pwm_resolution);
      ledcAttachPin(analogout_PIN_2, PWM_CHANNEL_analogout_2);
      ledcWrite(PWM_CHANNEL_analogout_2, 0);
#endif
    }
    else if (analogoutid == 3) {
      analogout_PIN_3 = analogoutpin;
      pinMode(analogout_PIN_3, OUTPUT);
      digitalWrite(analogout_PIN_3, LOW);
#ifdef IS_ESP32
      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_analogout_3, pwm_frequency, pwm_resolution);
      ledcAttachPin(analogout_PIN_3, PWM_CHANNEL_analogout_3);
      ledcWrite(PWM_CHANNEL_analogout_3, 0);
#endif
  }
}

jsonDocument.clear();
jsonDocument["return"] = 1;

}

// Custom function accessible by the API
void analogout_get_fct() {
  // GET SOME PARAMETERS HERE
  int analogoutid = jsonDocument["analogoutid"];
  int analogoutpin = 0;
  int analogoutval = 0;

  if (analogoutid == 1) {
    if (DEBUG) Serial.println("analogout 1");
    analogoutpin = analogout_PIN_1;
    analogoutval = analogout_val_1;
  }
  else if (analogoutid == 2) {
    if (DEBUG) Serial.println("AXIS 2");
    if (DEBUG) Serial.println("analogout 2");
    analogoutpin = analogout_PIN_2;
    analogoutval = analogout_val_2;
  }
  else if (analogoutid == 3) {
    if (DEBUG) Serial.println("AXIS 3");
    if (DEBUG) Serial.println("analogout 1");
    analogoutpin = analogout_PIN_3;
    analogoutval = analogout_val_3;
  }

  jsonDocument.clear();
  jsonDocument["analogoutid"] = analogoutid;
  jsonDocument["analogoutval"] = analogoutval;
  jsonDocument["analogoutpin"] = analogoutpin;
}


/*
  wrapper for HTTP requests
*/
#ifdef IS_WIFI
void analogout_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  analogout_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void analogout_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  analogout_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void analogout_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  analogout_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
