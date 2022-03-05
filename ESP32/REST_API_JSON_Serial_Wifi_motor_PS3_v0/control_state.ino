
#include "state_parameters.h"
#ifdef ARDUINO_SERIAL
#define IS_SERIAL
#define IS_ARDUINO
#endif

#ifdef ESP32_SERIAL
#define IS_SERIAL
#define IS_ESP32
#endif

#ifdef ESP32_WIFI
#define IS_WIFI
#define IS_ESP32
#endif

#ifdef ESP32_SERIAL_WIFI
#define IS_WIFI
#define IS_SERIAL
#define IS_ESP32
#endif

static inline int8_t sgn(int val) {
  if (val < 0) return -1;
  if (val == 0) return 0;
  return 1;
}
 
 // Custom function accessible by the API
void state_act_fct() {
  // here you can do something
  Serial.println("state_act_fct");
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void state_set_fct() {
  // here you can set parameters

  int isdebug = jsonDocument["isdebug"];
  DEBUG = isdebug;
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
  jsonDocument["identifier_setup"] = identifier_setup;
}

void printInfo(){
  Serial.println("You can use this software by sending JSON strings, A full documentation can be found here:");
  Serial.println("https://github.com/openUC2/UC2-REST/");
  //Serial.println("A first try can be: \{\"task\": \"/state_get\"");
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
