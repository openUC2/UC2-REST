#include "SensorController.h"

// Custom function accessible by the API
void SensorController::act() {

  // here you can do something
  if (DEBUG) Serial.println("readsensor_act_fct");
  int readsensorID = (int)(*jsonDocument)["readsensorID"];
  int mN_sensor_avg = N_sensor_avg;
  if (jsonDocument->containsKey("N_sensor_avg"))
    mN_sensor_avg = (int)(*jsonDocument)["N_sensor_avg"];
  int sensorpin = 0 ;

  if (DEBUG) Serial.print("readsensorID "); Serial.println(readsensorID);
  switch (readsensorID) {
    case 0:
      sensorpin = pins->ADC_pin_0;
      break;
    case 1:
      sensorpin = pins->ADC_pin_1;
      break;
    case 2:
      sensorpin = pins->ADC_pin_2;
      break;
  }

  float sensorValueAvg = 0;
  for (int imeas=0; imeas < N_sensor_avg; imeas++) {
    sensorValueAvg += analogRead(sensorpin);
  }
  float returnValue = (float)sensorValueAvg / (float)N_sensor_avg;

  jsonDocument->clear();
  (*jsonDocument)["sensorValue"] = returnValue;
  (*jsonDocument)["sensorpin"] = sensorpin;
  (*jsonDocument)["N_sensor_avg"] = N_sensor_avg;

}



void SensorController::get() {
  if (DEBUG) Serial.println("readsensor_set_fct");
  int readsensorID = (int)(*jsonDocument)["readsensorID"];
  int readsensorPIN = (int)(*jsonDocument)["readsensorPIN"];
  if (jsonDocument->containsKey("N_sensor_avg"))
    N_sensor_avg = (int)(*jsonDocument)["N_sensor_avg"];

  switch (readsensorID) {
    case 0:
      pins->ADC_pin_0 = readsensorPIN;
      break;
    case 1:
      pins->ADC_pin_1 = readsensorPIN;
      break;
    case 2:
      pins->ADC_pin_2 = readsensorPIN;
      break;
  }


  jsonDocument->clear();
  (*jsonDocument)["readsensorPIN"] = readsensorPIN;
  (*jsonDocument)["readsensorID"] = readsensorID;
}



// Custom function accessible by the API
void SensorController::set() {
if (DEBUG) Serial.println("readsensor_get_fct");
  int readsensorID = (int)(*jsonDocument)["readsensorID"];
  int readsensorPIN = 0;
  switch (readsensorID) {
    case 0:
      readsensorPIN = pins->ADC_pin_0;
      break;
    case 1:
      readsensorPIN = pins->ADC_pin_1;
      break;
    case 2:
      readsensorPIN = pins->ADC_pin_2;
      break;
  }

  jsonDocument->clear();
  (*jsonDocument)["N_sensor_avg"] = N_sensor_avg;
  (*jsonDocument)["readsensorPIN"] = readsensorPIN;
  (*jsonDocument)["readsensorID"] = readsensorID;
}


void SensorController::setup(){
  if(DEBUG) Serial.println("Setting up sensors...");
}

