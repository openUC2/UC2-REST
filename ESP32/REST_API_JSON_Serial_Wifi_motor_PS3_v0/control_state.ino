
 #include "state_parameters.h"
 
 // Custom function accessible by the API
void state_act_fct() {
  // here you can do something
  Serial.println("state_act_fct");
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void state_set_fct() {
  // here you can set parameters
  jsonDocument.clear();
  jsonDocument["return"] = 1;

}

// Custom function accessible by the API
void state_get_fct() {
  // GET SOME PARAMETERS HERE
  jsonDocument.clear();
  jsonDocument["identifier_name"] = identifier_name;
  jsonDocument["identifier_id"] = identifier_id;
  jsonDocument["identifier_date"] = identifier_date;
  jsonDocument["identifier_author"] = identifier_author;
}


/*
   wrapper for HTTP requests
*/
#ifdef IS_WIFI 
void state_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  state_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void state_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  state_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void state_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  state_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
