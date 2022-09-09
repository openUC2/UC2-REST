#include <Preferences.h>
#include <ArduinoJson.h>

Preferences preferences;
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
const char* keyMotorXStepPin = "motXstp";
const char* keyMotorXDirPin = "motXdir";
const char* keyMotorYStepPin = "motYstp";
const char* keyMotorYDirPin = "motYdir";
const char* keyMotorZStepPin = "motZstp";
const char* keyMotorZDirPin = "motZdir";
const char* keyMotorAStepPin = "motAstp";
const char* keyMotorADirPin = "motAdir";
const char* keyMotorEnable = "motEnable";

const char* keyLEDArray = "ledArrPin";
const char* keyLEDNumLEDArray = "ledArrNum";

const char* keyDigital1Pin = "digitalPin1";
const char* keyDigital2Pin = "digitalPin2";

const char* keyAnalog1Pin = "analogPin1";
const char* keyAnalog2Pin = "analogPin2";
const char* keyAnalog3Pin = "analogPin3";

const char* keyLaser1Pin = "laserPin1";
const char* keyLaser2Pin = "laserPin2";
const char* keyLaser3Pin = "laserPin3";

const char* keyDACfake1Pin = "dacFake1";
const char* keyDACfake2Pin = "dacFake2";

const char* keyIdentifier = "identifier";

const char* keyWifiSSID = "ssid";
const char* keyWifiPW = "PW";

String identifier_setup = "pindef_multicolour_wemos_lena";
String wifiSSID = "SSID";
String wifiPW = "PW";

void setup() {
  Serial.begin(115200);
  getPreferences();
}

bool resetPreferences() {
  preferences.begin(prefNamespace , false);
  preferences.clear();
  preferences.end();
  return true;
}

bool setPreferences() {
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


bool getPreferences() {

  preferences.begin(prefNamespace, false);
  STEP_X = preferences.getUInt(keyMotorXStepPin, STEP_X);
  DIR_X = preferences.getUInt(keyMotorXDirPin, DIR_X);
  Serial.println(STEP_X);
  Serial.println(DIR_X);

  STEP_Y = preferences.getUInt(keyMotorYStepPin, STEP_Y);
  DIR_Y = preferences.getUInt(keyMotorYDirPin, DIR_Y);
  Serial.println(STEP_Y);
  Serial.println(DIR_Y);

  STEP_Z = preferences.getUInt(keyMotorZStepPin, STEP_Z);
  DIR_Z = preferences.getUInt(keyMotorZDirPin, DIR_Z);
  Serial.println(STEP_Z);
  Serial.println(DIR_Z);

  STEP_A = preferences.getUInt(keyMotorAStepPin, STEP_A);
  DIR_A = preferences.getUInt(keyMotorADirPin, DIR_A);
  Serial.println(STEP_A);
  Serial.println(DIR_A);

  ENABLE = preferences.getUInt(keyMotorEnable, ENABLE);
  Serial.println(ENABLE);

  LED_ARRAY_PIN = preferences.getUInt(keyMotorEnable, LED_ARRAY_PIN);
  LED_ARRAY_NUM = preferences.getUInt(keyMotorEnable, LED_ARRAY_NUM);
  Serial.println(LED_ARRAY_PIN);
  Serial.println(LED_ARRAY_NUM);

  digital_PIN_1 = preferences.getUInt(keyDigital1Pin, digital_PIN_1);
  digital_PIN_2 = preferences.getUInt(keyDigital2Pin, digital_PIN_2);
  Serial.println(digital_PIN_1);
  Serial.println(digital_PIN_2);

  analog_PIN_1 = preferences.getUInt(keyAnalog1Pin, analog_PIN_1);
  analog_PIN_2 = preferences.getUInt(keyAnalog2Pin, analog_PIN_2);
  analog_PIN_3 = preferences.getUInt(keyAnalog3Pin, analog_PIN_3);
  Serial.println(analog_PIN_1);
  Serial.println(analog_PIN_2);
  Serial.println(analog_PIN_3);

  LASER_PIN_1 = preferences.getUInt(keyLaser1Pin, LASER_PIN_1);
  LASER_PIN_2 = preferences.getUInt(keyLaser2Pin, LASER_PIN_2);
  LASER_PIN_3 = preferences.getUInt(keyLaser3Pin, LASER_PIN_3);
  Serial.println(analog_PIN_1);
  Serial.println(analog_PIN_2);
  Serial.println(analog_PIN_3);

  dac_fake_1 = preferences.getUInt(keyDACfake1Pin, dac_fake_1);
  dac_fake_2 = preferences.getUInt(keyDACfake2Pin, dac_fake_2);
  Serial.println(dac_fake_1);
  Serial.println(dac_fake_2);

  identifier_setup = preferences.getString(keyDACfake1Pin, identifier_setup);
  wifiSSID = preferences.getString(keyWifiSSID, wifiSSID);
  wifiPW = preferences.getString(keyWifiPW, wifiPW);
  Serial.println(identifier_setup);
  Serial.println(wifiSSID);
  Serial.println(wifiPW);

  jsonDocument.clear();
  
  // Assign to JSON jsonDocumentument
  jsonDocument["motXstp"] = STEP_X;
  jsonDocument["motXdir"] = DIR_X;
  jsonDocument["motYstp"] = STEP_Y;
  jsonDocument["motYdir"] = DIR_Y;
  jsonDocument["motZstp"] = STEP_Z;
  jsonDocument["motZdir"] = DIR_Z;
  jsonDocument["motAstp"] = STEP_A;
  jsonDocument["motAdir"] = DIR_A;
  jsonDocument["motEnable"] = ENABLE;
  jsonDocument["ledArrPin"] = LED_ARRAY_PIN;
  jsonDocument["ledArrNum"] = LED_ARRAY_NUM;
  jsonDocument["digitalPin1"] = digital_PIN_1;
  jsonDocument["digitalPin2"] = digital_PIN_2;
  jsonDocument["analogPin1"] = analog_PIN_1;
  jsonDocument["analogPin2"] = analog_PIN_2;
  jsonDocument["analogPin3"] = analog_PIN_3;
  jsonDocument["laserPin1"] = LASER_PIN_1;
  jsonDocument["laserPin2"] = LASER_PIN_2;
  jsonDocument["laserPin3"] = LASER_PIN_3;
  jsonDocument["dacFake1"] = dac_fake_1;
  jsonDocument["dacFake2"] = dac_fake_2;
  jsonDocument["identifier"] = identifier_setup;
  jsonDocument["ssid"] = wifiSSID;
  jsonDocument["PW"] = wifiPW;

  serializeJson(jsonDocument, Serial);

  preferences.end();
  return true;

}

void loop() {
  if (Serial.available()) {
    DeserializationError error = deserializeJson(jsonDocument, Serial);

    if (error) {
      Serial.println(error.c_str());
    }
    else {
      setPreferences();
      getPreferences();
    }

  }
}
