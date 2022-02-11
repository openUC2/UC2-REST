
/*
  
 // Custom function accessible by the API
DynamicJsonDocument state_act_fct(JsonDocument& Values) {
  // here you can do something
  Serial.println("state_act_fct");

  int value = Values["value"];

  if (DEBUG) {
    Serial.print("value "); Serial.println(value);
  }

  Values.clear();
  Values["return"] = 1;

  return Values ;
}

DynamicJsonDocument state_set_fct(JsonDocument& Values) {
  // here you can set parameters
  int value = Values["value"];

  if (DEBUG) {
    Serial.print("value "); Serial.println(value);
  }

  int state_set = jsonDocument["state_set"];

  if (state_set != NULL) {
    if (DEBUG) Serial.print("state_set "); Serial.println(state_set);
    // SET SOMETHING
  }

  Values.clear();
  Values["return"] = 1;

  return Values ;
}

// Custom function accessible by the API
DynamicJsonDocument state_get_fct(JsonDocument& Values) {
  // GET SOME PARAMETERS HERE
  int state_variable = 12343;

  jsonDocument.clear();
  jsonDocument["state_variable"] = state_variable;
  return jsonDocument;
}

*/

/*
   wrapper for HTTP requests
*/
#ifdef IS_WIFI 
void state_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = state_act_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void state_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = state_get_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void state_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = state_set_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
