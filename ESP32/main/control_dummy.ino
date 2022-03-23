/*
 
 // Custom function accessible by the API
DynamicJsonDocument dummy_act_fct(JsonDocument& Values) {
  // here you can do something
  Serial.println("dummy_act_fct");

  int value = Values["value"];

  if (DEBUG) {
    Serial.print("value "); Serial.println(value);
  }

  Values.clear();
  Values["return"] = 1;

  return Values ;
}

DynamicJsonDocument dummy_set_fct(JsonDocument& Values) {
  // here you can set parameters
  int value = Values["value"];

  if (DEBUG) {
    Serial.print("value "); Serial.println(value);
  }

  int dummy_set = jsonDocument["dummy_set"];

  if (dummy_set != NULL) {
    if (DEBUG) Serial.print("dummy_set "); Serial.println(dummy_set);
    // SET SOMETHING
  }

  Values.clear();
  Values["return"] = 1;

  return Values ;
}

// Custom function accessible by the API
DynamicJsonDocument dummy_get_fct(JsonDocument& Values) {
  // GET SOME PARAMETERS HERE
  int dummy_variable = 12343;

  jsonDocument.clear();
  jsonDocument["dummy_variable"] = dummy_variable;
  return jsonDocument;
}


*
   wrapper for HTTP requests
*
#ifdef IS_WIFI 
void dummy_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = dummy_act_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void dummy_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = dummy_get_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void dummy_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = dummy_set_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
*/
