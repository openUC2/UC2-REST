#ifdef IS_READSENSOR

// Custom function accessible by the API
void readsensor_act_fct() {

  // here you can do something
  if (DEBUG) Serial.println("readsensor_act_fct");
  int readsensorID = (int)jsonDocument["readsensorID"];
  int mN_sensor_avg = N_sensor_avg;
  if (jsonDocument.containsKey("N_sensor_avg"))
    mN_sensor_avg = (int)jsonDocument["N_sensor_avg"];
  int sensorpin = 0 ;

  if (DEBUG) Serial.print("readsensorID "); Serial.println(readsensorID);
  switch (readsensorID) {
    case 0:
      sensorpin = ADC_pin_0;
      break;
    case 1:
      sensorpin = ADC_pin_1;
      break;
    case 2:
      sensorpin = ADC_pin_2;
      break;
  }

  float sensorValueAvg = 0;
  for (int imeas=0; imeas < N_sensor_avg; imeas++) {
    sensorValueAvg += analogRead(sensorpin);
  }
  float returnValue = (float)sensorValueAvg / (float)N_sensor_avg;

  jsonDocument.clear();
  jsonDocument["sensorValue"] = returnValue;
  jsonDocument["sensorpin"] = sensorpin;
  jsonDocument["N_sensor_avg"] = N_sensor_avg;

}



void readsensor_set_fct() {
  if (DEBUG) Serial.println("readsensor_set_fct");
  int readsensorID = (int)jsonDocument["readsensorID"];
  int readsensorPIN = (int)jsonDocument["readsensorPIN"];
  if (jsonDocument.containsKey("N_sensor_avg"))
    N_sensor_avg = (int)jsonDocument["N_sensor_avg"];

  switch (readsensorID) {
    case 0:
      ADC_pin_0 = readsensorPIN;
      break;
    case 1:
      ADC_pin_1 = readsensorPIN;
      break;
    case 2:
      ADC_pin_2 = readsensorPIN;
      break;
  }


  jsonDocument.clear();
  jsonDocument["readsensorPIN"] = readsensorPIN;
  jsonDocument["readsensorID"] = readsensorID;
}



// Custom function accessible by the API
void readsensor_get_fct() {
if (DEBUG) Serial.println("readsensor_get_fct");
  int readsensorID = (int)jsonDocument["readsensorID"];
  int readsensorPIN = 0;
  switch (readsensorID) {
    case 0:
      readsensorPIN = ADC_pin_0;
      break;
    case 1:
      readsensorPIN = ADC_pin_1;
      break;
    case 2:
      readsensorPIN = ADC_pin_2;
      break;
  }

  jsonDocument.clear();
  jsonDocument["N_sensor_avg"] = N_sensor_avg;
  jsonDocument["readsensorPIN"] = readsensorPIN;
  jsonDocument["readsensorID"] = readsensorID;
}


void setup_sensors(){
  if(DEBUG) Serial.println("Setting up sensors...");
}



/*
   wrapper for HTTP requests
*/
void readsensor_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  readsensor_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void readsensor_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  readsensor_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void readsensor_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  readsensor_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
