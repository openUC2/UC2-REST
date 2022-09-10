#include "ConfigController.h"


ConfigController::ConfigController(/* args */)
{
}

ConfigController::~ConfigController()
{
}


const char* prefNamespace = "UC2";

/*
  {
  "motXstp": 1,
  "motXdir": 2,
  "motYstp": 3,
  "motYdir": 4,
  "motZstp": 5,
  "motZdir": 6,
  "motAstp": 7,
  "motAdir": 8,
  "motEnable": 9,
  "ledArrPin": 0,
  "ledArrNum": 64,
  "digitalPin1":10,
  "digitalPin2":11,
  "analogPin1":12,
  "analogPin2":13,
  "analogPin3":14,
  "laserPin1":15,
  "laserPin2":16,
  "laserPin3":17,
  "dacFake1":18,
  "dacFake2":19,
  "identifier": "TEST",
  "ssid": "ssid",
  "PW": "PW"
  }
*/




void ConfigController::setup() {
  Serial.begin(115200);
  getPreferences();
}

bool ConfigController::resetPreferences() {
  preferences.begin(prefNamespace , false);
  preferences.clear();
  preferences.end();
  return true;
}

bool ConfigController::setPreferences() {
  preferences.begin(prefNamespace , false);

  preferences.putUInt(keyMotorXStepPin, (*jsonDocument)[keyMotorXStepPin]);
  preferences.putUInt(keyMotorXDirPin, (*jsonDocument)[keyMotorXDirPin]);

  preferences.putUInt(keyMotorYStepPin, (*jsonDocument)[keyMotorYStepPin]);
  preferences.putUInt(keyMotorYDirPin, (*jsonDocument)[keyMotorYDirPin]);

  preferences.putUInt(keyMotorZStepPin, (*jsonDocument)[keyMotorZStepPin]);
  preferences.putUInt(keyMotorZDirPin, (*jsonDocument)[keyMotorZDirPin]);

  preferences.putUInt(keyMotorAStepPin, (*jsonDocument)[keyMotorAStepPin]);
  preferences.putUInt(keyMotorADirPin, (*jsonDocument)[keyMotorADirPin]);

  preferences.putUInt(keyMotorEnable, (*jsonDocument)[keyMotorEnable]);

  preferences.putUInt(keyLEDArray, (*jsonDocument)[keyLEDArray]);
  preferences.putUInt(keyLEDNumLEDArray, (*jsonDocument)[keyLEDNumLEDArray]);

  preferences.putUInt(keyDigital1Pin, (*jsonDocument)[keyDigital1Pin]);
  preferences.putUInt(keyDigital2Pin, (*jsonDocument)[keyDigital2Pin]);

  preferences.putUInt(keyAnalog1Pin, (*jsonDocument)[keyAnalog1Pin]);
  preferences.putUInt(keyAnalog2Pin, (*jsonDocument)[keyAnalog2Pin]);
  preferences.putUInt(keyAnalog3Pin, (*jsonDocument)[keyAnalog3Pin]);

  preferences.putUInt(keyLaser1Pin, (*jsonDocument)[keyLaser1Pin]);
  preferences.putUInt(keyLaser2Pin, (*jsonDocument)[keyLaser2Pin]);
  preferences.putUInt(keyLaser3Pin, (*jsonDocument)[keyLaser3Pin]);

  preferences.putUInt(keyDACfake1Pin, (*jsonDocument)[keyDACfake1Pin]);
  preferences.putUInt(keyDACfake2Pin, (*jsonDocument)[keyDACfake2Pin]);

  preferences.putString(keyIdentifier,  (const char*)(*jsonDocument)[keyIdentifier]);
  preferences.putString(keyWifiSSID, (const char*)(*jsonDocument)[keyWifiSSID]);
  preferences.putString(keyWifiPW, (const char*)(*jsonDocument)[keyWifiPW]);

  preferences.end();
  return true;
}


bool ConfigController::getPreferences() {

  preferences.begin(prefNamespace, false);
  pins->STEP_X = preferences.getUInt(keyMotorXStepPin, pins->STEP_X);
  pins->DIR_X = preferences.getUInt(keyMotorXDirPin, pins->DIR_X);
  Serial.println(pins->STEP_X);
  Serial.println(pins->DIR_X);

  pins->STEP_Y = preferences.getUInt(keyMotorYStepPin, pins->STEP_Y);
  pins->DIR_Y = preferences.getUInt(keyMotorYDirPin, pins->DIR_Y);
  Serial.println(pins->STEP_Y);
  Serial.println(pins->DIR_Y);

  pins->STEP_Z = preferences.getUInt(keyMotorZStepPin, pins->STEP_Z);
  pins->DIR_Z = preferences.getUInt(keyMotorZDirPin, pins->DIR_Z);
  Serial.println(pins->STEP_Z);
  Serial.println(pins->DIR_Z);

  pins->STEP_A = preferences.getUInt(keyMotorAStepPin, pins->STEP_A);
  pins->DIR_A = preferences.getUInt(keyMotorADirPin, pins->DIR_A);
  Serial.println(pins->STEP_A);
  Serial.println(pins->DIR_A);

  pins->ENABLE = preferences.getUInt(keyMotorEnable, pins->ENABLE);
  Serial.println(pins->ENABLE);

  pins->LED_ARRAY_PIN = preferences.getUInt(keyMotorEnable, pins->LED_ARRAY_PIN);
  pins->LED_ARRAY_NUM = preferences.getUInt(keyMotorEnable, pins->LED_ARRAY_NUM);
  Serial.println(pins->LED_ARRAY_PIN);
  Serial.println(pins->LED_ARRAY_NUM);

  pins->digital_PIN_1 = preferences.getUInt(keyDigital1Pin, pins->digital_PIN_1);
  pins->digital_PIN_2 = preferences.getUInt(keyDigital2Pin, pins->digital_PIN_2);
  Serial.println(pins->digital_PIN_1);
  Serial.println(pins->digital_PIN_2);

  pins->analog_PIN_1 = preferences.getUInt(keyAnalog1Pin, pins->analog_PIN_1);
  pins->analog_PIN_2 = preferences.getUInt(keyAnalog2Pin, pins->analog_PIN_2);
  pins->analog_PIN_3 = preferences.getUInt(keyAnalog3Pin, pins->analog_PIN_3);
  Serial.println(pins->analog_PIN_1);
  Serial.println(pins->analog_PIN_2);
  Serial.println(pins->analog_PIN_3);

  pins->LASER_PIN_1 = preferences.getUInt(keyLaser1Pin, pins->LASER_PIN_1);
  pins->LASER_PIN_2 = preferences.getUInt(keyLaser2Pin, pins->LASER_PIN_2);
  pins->LASER_PIN_3 = preferences.getUInt(keyLaser3Pin, pins->LASER_PIN_3);
  Serial.println(pins->analog_PIN_1);
  Serial.println(pins->analog_PIN_2);
  Serial.println(pins->analog_PIN_3);

  pins->dac_fake_1 = preferences.getUInt(keyDACfake1Pin, pins->dac_fake_1);
  pins->dac_fake_2 = preferences.getUInt(keyDACfake2Pin, pins->dac_fake_2);
  Serial.println(pins->dac_fake_1);
  Serial.println(pins->dac_fake_2);

  identifier_setup = preferences.getString(keyDACfake1Pin, identifier_setup);
  wifiSSID = preferences.getString(keyWifiSSID, wifiSSID);
  wifiPW = preferences.getString(keyWifiPW, wifiPW);
  Serial.println(identifier_setup);
  Serial.println(wifiSSID);
  Serial.println(wifiPW);

  jsonDocument->clear();
  
  // Assign to JSON jsonDocumentument
  (*jsonDocument)["motXstp"] = pins->STEP_X;
  (*jsonDocument)["motXdir"] = pins->DIR_X;
  (*jsonDocument)["motYstp"] = pins->STEP_Y;
  (*jsonDocument)["motYdir"] = pins->DIR_Y;
  (*jsonDocument)["motZstp"] = pins->STEP_Z;
  (*jsonDocument)["motZdir"] = pins->DIR_Z;
  (*jsonDocument)["motAstp"] = pins->STEP_A;
  (*jsonDocument)["motAdir"] = pins->DIR_A;
  (*jsonDocument)["motEnable"] = pins->ENABLE;
  (*jsonDocument)["ledArrPin"] = pins->LED_ARRAY_PIN;
  (*jsonDocument)["ledArrNum"] = pins->LED_ARRAY_NUM;
  (*jsonDocument)["digitalPin1"] = pins->digital_PIN_1;
  (*jsonDocument)["digitalPin2"] = pins->digital_PIN_2;
  (*jsonDocument)["analogPin1"] = pins->analog_PIN_1;
  (*jsonDocument)["analogPin2"] = pins->analog_PIN_2;
  (*jsonDocument)["analogPin3"] = pins->analog_PIN_3;
  (*jsonDocument)["laserPin1"] = pins->LASER_PIN_1;
  (*jsonDocument)["laserPin2"] = pins->LASER_PIN_2;
  (*jsonDocument)["laserPin3"] = pins->LASER_PIN_3;
  (*jsonDocument)["dacFake1"] = pins->dac_fake_1;
  (*jsonDocument)["dacFake2"] = pins->dac_fake_2;
  (*jsonDocument)["identifier"] = identifier_setup;
  (*jsonDocument)["ssid"] = wifiSSID;
  (*jsonDocument)["PW"] = wifiPW;

  serializeJson(*jsonDocument, Serial);

  preferences.end();
  return true;

}

void ConfigController::loop() {
  if (Serial.available()) {
    DeserializationError error = deserializeJson(*jsonDocument, Serial);

    if (error) {
      Serial.println(error.c_str());
    }
    else {
      setPreferences();
      getPreferences();
    }
  }
}
