#ifdef IS_DIGITAL

// Custom function accessible by the API
void digital_act_fct() {
  // here you can do something
  Serial.println("digital_act_fct");

  int digitalid = jsonDocument["digitalid"];
  int digitalval = jsonDocument["digitalval"];

  if (DEBUG) {
    Serial.print("digitalid "); Serial.println(digitalid);
    Serial.print("digitalval "); Serial.println(digitalval);
  }

  if (digitalid == 1) {
    digital_val_1 = digitalval;
    digitalWrite(digital_PIN_1, digital_val_1);
  }
  else if (digitalid == 2) {
    digital_val_2 = digitalval;
    digitalWrite(digital_PIN_3, digital_val_2);
  }
  else if (digitalid == 3) {
    digital_val_3 = digitalval;
    digitalWrite(digital_PIN_3, digital_val_3);
  }
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void digital_set_fct() {
  // here you can set parameters
  int digitalid = jsonDocument["digitalid"];
  int digitalpin = jsonDocument["digitalpin"];

  if (DEBUG) Serial.print("digitalid "); Serial.println(digitalid);
  if (DEBUG) Serial.print("digitalpin "); Serial.println(digitalpin);

  if (digitalid != NULL and digitalpin != NULL) {
    if (digitalid == 1) {
      digital_PIN_1 = digitalpin;
      pinMode(digital_PIN_1, OUTPUT);
      digitalWrite(digital_PIN_1, LOW);
    }
    else if (digitalid == 2) {
      digital_PIN_2 = digitalpin;
      pinMode(digital_PIN_2, OUTPUT);
      digitalWrite(digital_PIN_2, LOW);
    }
    else if (digitalid == 3) {
      digital_PIN_3 = digitalpin;
      pinMode(digital_PIN_3, OUTPUT);
      digitalWrite(digital_PIN_3, LOW);
  }
}

jsonDocument.clear();
jsonDocument["return"] = 1;

}

// Custom function accessible by the API
void digital_get_fct() {
  // GET SOME PARAMETERS HERE
  int digitalid = jsonDocument["digitalid"];
  int digitalpin = 0;
  int digitalval = 0;

  if (digitalid == 1) {
    if (DEBUG) Serial.println("digital 1");
    digitalpin = digital_PIN_1;
    digitalval = digital_val_1;
  }
  else if (digitalid == 2) {
    if (DEBUG) Serial.println("AXIS 2");
    if (DEBUG) Serial.println("digital 2");
    digitalpin = digital_PIN_2;
    digitalval = digital_val_2;
  }
  else if (digitalid == 3) {
    if (DEBUG) Serial.println("AXIS 3");
    if (DEBUG) Serial.println("digital 1");
    digitalpin = digital_PIN_3;
    digitalval = digital_val_3;
  }

  jsonDocument.clear();
  jsonDocument["digitalid"] = digitalid;
  jsonDocument["digitalval"] = digitalval;
  jsonDocument["digitalpin"] = digitalpin;
}


/*
  wrapper for HTTP requests
*/
#ifdef IS_WIFI
void digital_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  digital_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void digital_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  digital_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void digital_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  digital_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
