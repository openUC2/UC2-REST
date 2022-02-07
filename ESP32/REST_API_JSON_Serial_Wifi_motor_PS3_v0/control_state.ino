
 #include "state_parameters.h"
 
 // Custom function accessible by the API
DynamicJsonDocument state_act_fct(JsonDocument& Values) {
  // here you can do something
  Serial.println("state_act_fct");
  Values.clear();
  Values["return"] = 1;

  return Values ;
}

DynamicJsonDocument state_set_fct(JsonDocument& Values) {
  // here you can set parameters
  Values.clear();
  Values["return"] = 1;

  return Values ;
}

// Custom function accessible by the API
DynamicJsonDocument state_get_fct(JsonDocument& Values) {
  // GET SOME PARAMETERS HERE

  
  
  jsonDocument.clear();
  jsonDocument["identifier_name"] = identifier_name;
  jsonDocument["identifier_id"] = identifier_id;
  jsonDocument["identifier_date"] = identifier_date;
  jsonDocument["identifier_author"] = identifier_author;
  return jsonDocument;
}


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
