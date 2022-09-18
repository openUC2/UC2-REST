#include "SensorController.h"

SensorController::SensorController(/* args */){};
SensorController::~SensorController(){};

// Custom function accessible by the API
void SensorController::act() {

  // here you can do something
  if (DEBUG) Serial.println("readsensor_act_fct");
  int readsensorID = (int)(*WifiController::getJDoc())["readsensorID"];
  int mN_sensor_avg = N_sensor_avg;
  if (WifiController::getJDoc()->containsKey("N_sensor_avg"))
    mN_sensor_avg = (int)(*WifiController::getJDoc())["N_sensor_avg"];
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

  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["sensorValue"] = returnValue;
  (*WifiController::getJDoc())["sensorpin"] = sensorpin;
  (*WifiController::getJDoc())["N_sensor_avg"] = N_sensor_avg;

}



void SensorController::get() {
  if (DEBUG) Serial.println("readsensor_set_fct");
  int readsensorID = (int)(*WifiController::getJDoc())["readsensorID"];
  int readsensorPIN = (int)(*WifiController::getJDoc())["readsensorPIN"];
  if (WifiController::getJDoc()->containsKey("N_sensor_avg"))
    N_sensor_avg = (int)(*WifiController::getJDoc())["N_sensor_avg"];

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


  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["readsensorPIN"] = readsensorPIN;
  (*WifiController::getJDoc())["readsensorID"] = readsensorID;
}



// Custom function accessible by the API
void SensorController::set() {
if (DEBUG) Serial.println("readsensor_get_fct");
  int readsensorID = (int)(*WifiController::getJDoc())["readsensorID"];
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

  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["N_sensor_avg"] = N_sensor_avg;
  (*WifiController::getJDoc())["readsensorPIN"] = readsensorPIN;
  (*WifiController::getJDoc())["readsensorID"] = readsensorID;
}


void SensorController::setup(PINDEF *pins){
  this->pins = pins;
  if(DEBUG) Serial.println("Setting up sensors...");
}

