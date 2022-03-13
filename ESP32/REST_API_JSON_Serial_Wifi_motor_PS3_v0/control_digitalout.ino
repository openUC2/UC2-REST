#ifdef IS_DIGITALOUT

// Custom function accessible by the API
void digitalout_act_fct() {
  // here you can do something
  Serial.println("digitalout_act_fct");

  int digitaloutid = jsonDocument["digitaloutid"];
  int digitaloutval = jsonDocument["digitaloutval"];

  if (DEBUG) {
    Serial.print("digitaloutid "); Serial.println(digitaloutid);
    Serial.print("digitaloutval "); Serial.println(digitaloutval);
  }

  if (digitaloutid == 1) {
    digitalout_val_1 = digitaloutval;
    digitalWrite(digitalout_PIN_1, digitalout_val_1);
  }
  else if (digitaloutid == 2) {
    digitalout_val_2 = digitaloutval;
    digitalWrite(digitalout_PIN_3, digitalout_val_2);
  }
  else if (digitaloutid == 3) {
    digitalout_val_3 = digitaloutval;
    digitalWrite(digitalout_PIN_3, digitalout_val_3);
  }
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void digitalout_set_fct() {
  // here you can set parameters
  int digitaloutid = jsonDocument["digitaloutid"];
  int digitaloutpin = jsonDocument["digitaloutpin"];

  if (DEBUG) Serial.print("digitaloutid "); Serial.println(digitaloutid);
  if (DEBUG) Serial.print("digitaloutpin "); Serial.println(digitaloutpin);

  if (digitaloutid != NULL and digitaloutpin != NULL) {
    if (digitaloutid == 1) {
      digitalout_PIN_1 = digitaloutpin;
      pinMode(digitalout_PIN_1, OUTPUT);
      digitalWrite(digitalout_PIN_1, LOW);
    }
    else if (digitaloutid == 2) {
      digitalout_PIN_2 = digitaloutpin;
      pinMode(digitalout_PIN_2, OUTPUT);
      digitalWrite(digitalout_PIN_2, LOW);
    }
    else if (digitaloutid == 3) {
      digitalout_PIN_3 = digitaloutpin;
      pinMode(digitalout_PIN_3, OUTPUT);
      digitalWrite(digitalout_PIN_3, LOW);
  }
}

jsonDocument.clear();
jsonDocument["return"] = 1;

}

// Custom function accessible by the API
void digitalout_get_fct() {
  // GET SOME PARAMETERS HERE
  int digitaloutid = jsonDocument["digitaloutid"];
  int digitaloutpin = 0;
  int digitaloutval = 0;

  if (digitaloutid == 1) {
    if (DEBUG) Serial.println("digitalout 1");
    digitaloutpin = digitalout_PIN_1;
    digitaloutval = digitalout_val_1;
  }
  else if (digitaloutid == 2) {
    if (DEBUG) Serial.println("AXIS 2");
    if (DEBUG) Serial.println("digitalout 2");
    digitaloutpin = digitalout_PIN_2;
    digitaloutval = digitalout_val_2;
  }
  else if (digitaloutid == 3) {
    if (DEBUG) Serial.println("AXIS 3");
    if (DEBUG) Serial.println("digitalout 1");
    digitaloutpin = digitalout_PIN_3;
    digitaloutval = digitalout_val_3;
  }

  jsonDocument.clear();
  jsonDocument["digitaloutid"] = digitaloutid;
  jsonDocument["digitaloutval"] = digitaloutval;
  jsonDocument["digitaloutpin"] = digitaloutpin;
}


/*
  wrapper for HTTP requests
*/
#ifdef IS_WIFI
void digitalout_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  digitalout_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void digitalout_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  digitalout_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void digitalout_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  digitalout_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
