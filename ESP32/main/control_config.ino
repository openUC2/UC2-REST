#include <Preferences.h>
#include <ArduinoJson.h>
#include "parameters_config.h"

Preferences preferences;



void config_act_fct() {

  if(jsonDocument.containsKey("resetPrefs")) {
    resetPreferences();
  }
  
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void config_set_fct() {
  setPreferences();
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void config_get_fct() {
  loadPreferences();
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

bool resetPreferences() {
  preferences.begin(prefNamespace , false);
  preferences.clear();
  preferences.end();
  return true;
}

bool setPreferences() {
  /*
  This function sets the preferences for the parameters based on a JSON document.
  */
  preferences.begin(prefNamespace , false);

  preferences.putUInt(keyMotorXStepPin, jsonDocument[keyMotorXStepPin]);
  preferences.putUInt(keyMotorXDirPin, jsonDocument[keyMotorXDirPin]);

  preferences.putUInt(keyMotorYStepPin, jsonDocument[keyMotorYStepPin]);
  preferences.putUInt(keyMotorYDirPin, jsonDocument[keyMotorYDirPin]);

  preferences.putUInt(keyMotorZStepPin, jsonDocument[keyMotorZStepPin]);
  preferences.putUInt(keyMotorZDirPin, jsonDocument[keyMotorZDirPin]);

  preferences.putUInt(keyMotorAStepPin, jsonDocument[keyMotorAStepPin]);
  preferences.putUInt(keyMotorADirPin, jsonDocument[keyMotorADirPin]);

  preferences.putUInt(keyMotorEnable, jsonDocument[keyMotorEnable]);

  preferences.putUInt(keyLEDArray, jsonDocument[keyLEDArray]);
  preferences.putUInt(keyLEDNumLEDArray, jsonDocument[keyLEDNumLEDArray]);

  preferences.putUInt(keyDigital1Pin, jsonDocument[keyDigital1Pin]);
  preferences.putUInt(keyDigital2Pin, jsonDocument[keyDigital2Pin]);

  preferences.putUInt(keyAnalog1Pin, jsonDocument[keyAnalog1Pin]);
  preferences.putUInt(keyAnalog2Pin, jsonDocument[keyAnalog2Pin]);
  preferences.putUInt(keyAnalog3Pin, jsonDocument[keyAnalog3Pin]);

  preferences.putUInt(keyLaser1Pin, jsonDocument[keyLaser1Pin]);
  preferences.putUInt(keyLaser2Pin, jsonDocument[keyLaser2Pin]);
  preferences.putUInt(keyLaser3Pin, jsonDocument[keyLaser3Pin]);

  preferences.putUInt(keyDACfake1Pin, jsonDocument[keyDACfake1Pin]);
  preferences.putUInt(keyDACfake2Pin, jsonDocument[keyDACfake2Pin]);

  String Identifier = jsonDocument[keyIdentifier];
  String WifiSSID = jsonDocument[keyWifiSSID];
  String WifiPW = jsonDocument[keyWifiPW];
  preferences.putString(keyIdentifier, Identifier);
  preferences.putString(keyWifiSSID, WifiSSID);
  preferences.putString(keyWifiPW, WifiPW);

  preferences.end();
  return true;
}


bool loadPreferences() {
  /* This function gets the preferences for the parameters
  and returns a JSON document. */

  preferences.begin(prefNamespace, false);
  STEP_PIN_X = preferences.getUInt(keyMotorXStepPin, STEP_PIN_X);
  DIR_PIN_X = preferences.getUInt(keyMotorXDirPin, DIR_PIN_X);

  STEP_PIN_Y = preferences.getUInt(keyMotorYStepPin, STEP_PIN_Y);
  DIR_PIN_Y = preferences.getUInt(keyMotorYDirPin, DIR_PIN_Y);

  STEP_PIN_Z = preferences.getUInt(keyMotorZStepPin, STEP_PIN_Z);
  DIR_PIN_Z = preferences.getUInt(keyMotorZDirPin, DIR_PIN_Z);

  STEP_PIN_A = preferences.getUInt(keyMotorAStepPin, STEP_PIN_A);
  DIR_PIN_A = preferences.getUInt(keyMotorADirPin, DIR_PIN_A);

  ENABLE_PIN = preferences.getUInt(keyMotorEnable, ENABLE_PIN);

  LED_ARRAY_PIN = preferences.getUInt(keyMotorEnable, LED_ARRAY_PIN);
  LED_ARRAY_NUM = preferences.getUInt(keyMotorEnable, LED_ARRAY_NUM);

  DIGITAL_PIN_1 = preferences.getUInt(keyDigital1Pin, DIGITAL_PIN_1);
  DIGITAL_PIN_2 = preferences.getUInt(keyDigital2Pin, DIGITAL_PIN_2);

  ANALOG_PIN_1 = preferences.getUInt(keyAnalog1Pin, ANALOG_PIN_1);
  ANALOG_PIN_2 = preferences.getUInt(keyAnalog2Pin, ANALOG_PIN_2);
  ANALOG_PIN_3 = preferences.getUInt(keyAnalog3Pin, ANALOG_PIN_3);

  LASER_PIN_1 = preferences.getUInt(keyLaser1Pin, LASER_PIN_1);
  LASER_PIN_2 = preferences.getUInt(keyLaser2Pin, LASER_PIN_2);
  LASER_PIN_3 = preferences.getUInt(keyLaser3Pin, LASER_PIN_3);

  DAC_FAKE_PIN_1 = preferences.getUInt(keyDACfake1Pin, DAC_FAKE_PIN_1);
  DAC_FAKE_PIN_2 = preferences.getUInt(keyDACfake2Pin, DAC_FAKE_PIN_2);

  IDENTIFIER_NAME = preferences.getString(keyDACfake1Pin, IDENTIFIER_NAME);
  wifiSSID = preferences.getString(keyWifiSSID, wifiSSID);
  wifiPW = preferences.getString(keyWifiPW, WifiPW);

  jsonDocument.clear();
  
  // Assign to JSON jsonDocumentument
  jsonDocument["motXstp"] = STEP_PIN_X;
  jsonDocument["motXdir"] = DIR_PIN_X;
  jsonDocument["motYstp"] = STEP_PIN_Y;
  jsonDocument["motYdir"] = DIR_PIN_Y;
  jsonDocument["motZstp"] = STEP_PIN_Z;
  jsonDocument["motZdir"] = DIR_PIN_Z;
  jsonDocument["motAstp"] = STEP_PIN_A;
  jsonDocument["motAdir"] = DIR_PIN_A;
  jsonDocument["motEnable"] = ENABLE_PIN;
  jsonDocument["ledArrPin"] = LED_ARRAY_PIN;
  jsonDocument["ledArrNum"] = LED_ARRAY_NUM;
  jsonDocument["digitalPin1"] = DIGITAL_PIN_1;
  jsonDocument["digitalPin2"] = DIGITAL_PIN_2;
  jsonDocument["analogPin1"] = ANALOG_PIN_1;
  jsonDocument["analogPin2"] = ANALOG_PIN_2;
  jsonDocument["analogPin3"] = ANALOG_PIN_3;
  jsonDocument["laserPin1"] = LASER_PIN_1;
  jsonDocument["laserPin2"] = LASER_PIN_2;
  jsonDocument["laserPin3"] = LASER_PIN_3;
  jsonDocument["dacFake1"] = DAC_FAKE_PIN_1;
  jsonDocument["dacFake2"] = DAC_FAKE_PIN_2;
  jsonDocument["identifier"] = IDENTIFIER_NAME;
  jsonDocument["ssid"] = wifiSSID;
  jsonDocument["PW"] = wifiPW;

  Serial.println("Current pin definitions:");
  serializeJsonPretty(jsonDocument, Serial);

  preferences.end();
  return true;
}

