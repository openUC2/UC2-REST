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
  if (DEBUG) Serial.println("state_act_fct");

  // assign default values to thhe variables
  if (jsonDocument.containsKey("delay")) {
    int mdelayms = jsonDocument["delay"];
    delay(mdelayms);
  }
  if (jsonDocument.containsKey("isBusy")) {
    isBusy = jsonDocument["isBusy"];
  }
  if (jsonDocument.containsKey("pscontroller")) {
    IS_PSCONTROLER_ACTIVE = jsonDocument["pscontroller"];
  }




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
  if (jsonDocument.containsKey("active")) {
    jsonDocument.clear();
    jsonDocument["isBusy"] = isBusy; // returns state of function that takes longer to finalize (e.g. motor)
  }
  else if (jsonDocument.containsKey("pscontroller")) {
    jsonDocument.clear();
    jsonDocument["pscontroller"] = IS_PSCONTROLER_ACTIVE; // returns state of function that takes longer to finalize (e.g. motor)
  }
  else {
    jsonDocument.clear();
    jsonDocument["identifier_name"] = identifier_name;
    jsonDocument["identifier_id"] = identifier_id;
    jsonDocument["identifier_date"] = identifier_date;
    jsonDocument["identifier_author"] = identifier_author;
    jsonDocument["identifier_setup"] = identifier_setup;
  }
}

void printInfo() {
  if (DEBUG) Serial.println("You can use this software by sending JSON strings, A full documentation can be found here:");
  if (DEBUG) Serial.println("https://github.com/openUC2/UC2-REST/");
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
