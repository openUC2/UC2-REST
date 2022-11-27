#ifdef IS_ANALOG

// Custom function accessible by the API
void analog_act_fct() {
  // here you can do something
  Serial.println("analog_act_fct");

  int analogid = jsonDocument["analogid"];
  int analogval = jsonDocument["analogval"];

  if (DEBUG) {
    Serial.print("analogid "); Serial.println(analogid);
    Serial.print("analogval "); Serial.println(analogval);
  }

  if (analogid == 1) {
    analog_val_1 = analogval;
    ledcWrite(PWM_CHANNEL_analog_1, analogval);
  }
  else if (analogid == 2) {
    analog_val_2 = analogval;
    ledcWrite(PWM_CHANNEL_analog_2, analogval);
  }
  else if (analogid == 3) {
    analog_val_3 = analogval;
    ledcWrite(PWM_CHANNEL_analog_3, analogval);
  }
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void analog_set_fct() {
  // here you can set parameters
  
  if (jsonDocument.containsKey("analogid")) {
    int analogid = jsonDocument["analogid"];
  }
  else{
    int analogid = 0;
  }

  if (jsonDocument.containsKey("analogpin")) {
    int analogpin = jsonDocument["analogpin"];
  }
  else{
    int analogpin = 0;
  }

  if (DEBUG) Serial.print("analogid "); Serial.println(analogid);
  if (DEBUG) Serial.print("analogpin "); Serial.println(analogpin);



    if (analogid == 1) {
      ANALOG_PIN_1 = analogpin;
      pinMode(ANALOG_PIN_1, OUTPUT);
      digitalWrite(ANALOG_PIN_1, LOW);

      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_analog_1, pwm_frequency, pwm_resolution);
      ledcAttachPin(ANALOG_PIN_1, PWM_CHANNEL_analog_1);
      ledcWrite(PWM_CHANNEL_analog_1, 0);

    }
    else if (analogid == 2) {
      ANALOG_PIN_2 = analogpin;
      pinMode(ANALOG_PIN_2, OUTPUT);
      digitalWrite(ANALOG_PIN_2, LOW);

      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_analog_2, pwm_frequency, pwm_resolution);
      ledcAttachPin(ANALOG_PIN_2, PWM_CHANNEL_analog_2);
      ledcWrite(PWM_CHANNEL_analog_2, 0);
    }
    else if (analogid == 3) {
      ANALOG_PIN_3 = analogpin;
      pinMode(ANALOG_PIN_3, OUTPUT);
      digitalWrite(ANALOG_PIN_3, LOW);

      /* setup the PWM ports and reset them to 0*/
      ledcSetup(PWM_CHANNEL_analog_3, pwm_frequency, pwm_resolution);
      ledcAttachPin(ANALOG_PIN_3, PWM_CHANNEL_analog_3);
      ledcWrite(PWM_CHANNEL_analog_3, 0);

}

jsonDocument.clear();
jsonDocument["return"] = 1;

}

// Custom function accessible by the API
void analog_get_fct() {
  // GET SOME PARAMETERS HERE
  int analogid = jsonDocument["analogid"];
  int analogpin = 0;
  int analogval = 0;

  if (analogid == 1) {
    if (DEBUG) Serial.println("analog 1");
    analogpin = ANALOG_PIN_1;
    analogval = analog_val_1;
  }
  else if (analogid == 2) {
    if (DEBUG) Serial.println("AXIS 2");
    if (DEBUG) Serial.println("analog 2");
    analogpin = ANALOG_PIN_2;
    analogval = analog_val_2;
  }
  else if (analogid == 3) {
    if (DEBUG) Serial.println("AXIS 3");
    if (DEBUG) Serial.println("analog 1");
    analogpin = ANALOG_PIN_3;
    analogval = analog_val_3;
  }

  jsonDocument.clear();
  jsonDocument["analogid"] = analogid;
  jsonDocument["analogval"] = analogval;
  jsonDocument["analogpin"] = analogpin;
}


/*
  wrapper for HTTP requests
*/
void analog_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  analog_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void analog_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  analog_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void analog_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  analog_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
