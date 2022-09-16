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


void ConfigController::setJsonToPref(const char * key)
{
  if (jsonDocument->containsKey(key))
    preferences.putUInt(key, (*jsonDocument)[key]);
}

void ConfigController::setPrefToPins(const char * key, int* val)
{
  *val = preferences.getUInt(key,*val);
}

void ConfigController::setPinsToJson(const char * key, int val)
{
  (*jsonDocument)[key] = val;
}

void ConfigController::setup(PINDEF * pins, DynamicJsonDocument * jsonDocument) {
  this->pins = pins;
  this->jsonDocument = jsonDocument;
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

  setJsonToPref(keyMotorXStepPin);
  setJsonToPref(keyMotorXDirPin);

  setJsonToPref(keyMotorYStepPin);
  setJsonToPref(keyMotorYDirPin);

  setJsonToPref(keyMotorZStepPin);
  setJsonToPref(keyMotorZDirPin);

  setJsonToPref(keyMotorAStepPin);
  setJsonToPref(keyMotorADirPin);

  setJsonToPref(keyMotorEnable);

  setJsonToPref(keyLEDArray);
  setJsonToPref(keyLEDNumLEDArray);

  setJsonToPref(keyDigital1Pin);
  setJsonToPref(keyDigital2Pin);

  setJsonToPref(keyAnalog1Pin);
  setJsonToPref(keyAnalog2Pin);
  setJsonToPref(keyAnalog3Pin);

  setJsonToPref(keyLaser1Pin);
  setJsonToPref(keyLaser2Pin);
  setJsonToPref(keyLaser3Pin);

  setJsonToPref(keyDACfake1Pin);
  setJsonToPref(keyDACfake2Pin);

  preferences.putString(keyIdentifier,  (const char*)(*jsonDocument)[keyIdentifier]);
  preferences.putString(keyWifiSSID, (const char*)(*jsonDocument)[keyWifiSSID]);
  preferences.putString(keyWifiPW, (const char*)(*jsonDocument)[keyWifiPW]);
  preferences.putBool(keyWifiAP, (const char*)(*jsonDocument)[keyWifiAP]);

  preferences.end();
  return true;
}


bool ConfigController::getPreferences() {

  preferences.begin(prefNamespace, false);
  setPrefToPins(keyMotorXStepPin, &pins->STEP_X);
  setPrefToPins(keyMotorXDirPin, &pins->DIR_X);
  Serial.println(pins->STEP_X);
  Serial.println(pins->DIR_X);

  setPrefToPins(keyMotorYStepPin, &pins->STEP_Y);
  setPrefToPins(keyMotorYDirPin, &pins->DIR_Y); 
  Serial.println(pins->STEP_Y);
  Serial.println(pins->DIR_Y);

  setPrefToPins(keyMotorZStepPin, &pins->STEP_Z);
  setPrefToPins(keyMotorZDirPin, &pins->DIR_Z); 
  Serial.println(pins->STEP_Z);
  Serial.println(pins->DIR_Z);

  setPrefToPins(keyMotorAStepPin, &pins->STEP_A);
  setPrefToPins(keyMotorADirPin, &pins->DIR_A); 
  Serial.println(pins->STEP_A);
  Serial.println(pins->DIR_A);

  setPrefToPins(keyMotorEnable, &pins->ENABLE); 
  Serial.println(pins->ENABLE);

  setPrefToPins(keyLEDArray, &pins->LED_ARRAY_PIN);
  setPrefToPins(keyLEDNumLEDArray, &pins->LED_ARRAY_NUM); 
  Serial.println(pins->LED_ARRAY_PIN);
  Serial.println(pins->LED_ARRAY_NUM);

  setPrefToPins(keyDigital1Pin, &pins->digital_PIN_1);
  setPrefToPins(keyDigital2Pin, &pins->digital_PIN_2); 
  Serial.println(pins->digital_PIN_1);
  Serial.println(pins->digital_PIN_2);

  setPrefToPins(keyAnalog1Pin, &pins->analog_PIN_1);
  setPrefToPins(keyAnalog2Pin, &pins->analog_PIN_2);
  setPrefToPins(keyAnalog3Pin, &pins->analog_PIN_3);
  Serial.println(pins->analog_PIN_1);
  Serial.println(pins->analog_PIN_2);
  Serial.println(pins->analog_PIN_3);

  setPrefToPins(keyLaser1Pin, &pins->LASER_PIN_1);
  setPrefToPins(keyLaser2Pin, &pins->LASER_PIN_2);
  setPrefToPins(keyLaser3Pin, &pins->LASER_PIN_3);
  Serial.println(pins->analog_PIN_1);
  Serial.println(pins->analog_PIN_2);
  Serial.println(pins->analog_PIN_3);

  setPrefToPins(keyDACfake1Pin, &pins->dac_fake_1);
  setPrefToPins(keyDACfake2Pin, &pins->dac_fake_2);
  Serial.println(pins->dac_fake_1);
  Serial.println(pins->dac_fake_2);

  pins->identifier_setup = preferences.getString(keyIdentifier, pins->identifier_setup).c_str();
  pins->mSSID = preferences.getString(keyWifiSSID, pins->mSSID).c_str();
  pins->mPWD = preferences.getString(keyWifiPW, pins->mPWD).c_str();
  pins->mAP = preferences.getBool(keyWifiPW, pins->mAP);
  Serial.println(pins->identifier_setup);
  Serial.println(pins->mSSID);
  Serial.println(pins->mPWD);

  jsonDocument->clear();
  
  // Assign to JSON jsonDocumentument
  setPinsToJson(keyMotorXStepPin, pins->STEP_X);
  setPinsToJson(keyMotorXDirPin, pins->DIR_X);

  setPinsToJson(keyMotorYStepPin, pins->STEP_Y);
  setPinsToJson(keyMotorYDirPin, pins->DIR_Y);

  setPinsToJson(keyMotorZStepPin, pins->STEP_Z);
  setPinsToJson(keyMotorZDirPin, pins->DIR_Z);

  setPinsToJson(keyMotorAStepPin, pins->STEP_A);
  setPinsToJson(keyMotorADirPin, pins->DIR_A);

  setPinsToJson(keyMotorEnable, pins->ENABLE);

  setPinsToJson(keyLEDArray, pins->LED_ARRAY_PIN);
  setPinsToJson(keyLEDNumLEDArray, pins->LED_ARRAY_NUM);

  setPinsToJson(keyDigital1Pin, pins->digital_PIN_1);
  setPinsToJson(keyDigital2Pin, pins->digital_PIN_2);

  setPinsToJson(keyAnalog1Pin, pins->analog_PIN_1);
  setPinsToJson(keyAnalog2Pin, pins->analog_PIN_2);
  setPinsToJson(keyAnalog3Pin, pins->analog_PIN_3);

  setPinsToJson(keyLaser1Pin, pins->LASER_PIN_1);
  setPinsToJson(keyLaser2Pin, pins->LASER_PIN_2);
  setPinsToJson(keyLaser3Pin, pins->LASER_PIN_3);

  setPinsToJson(keyDACfake1Pin, pins->dac_fake_1);
  setPinsToJson(keyDACfake2Pin, pins->dac_fake_2);

  (*jsonDocument)[keyIdentifier] = pins->identifier_setup;
  (*jsonDocument)[keyWifiSSID] = pins->mSSID;
  (*jsonDocument)[keyWifiPW] = pins->mPWD;

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

void ConfigController::act()
{
    resetPreferences();
    jsonDocument->clear();
    (*jsonDocument)["return"] = 1;
}

void ConfigController::set()
{
    setPreferences();
    jsonDocument->clear();
    (*jsonDocument)["return"] = 1;
}

void ConfigController::get()
{
    getPreferences();
    jsonDocument->clear();
    (*jsonDocument)["return"] = 1;
}

bool ConfigController::isFirstRun() {
  preferences.begin(prefNamespace, false);
  // define preference name
  const char* prefName = "firstRun";
  preferences.begin(prefName, false);
  const char * dateKey = "date";
  const char *compiled_date = __DATE__ " " __TIME__;
  String stored_date = preferences.getString(dateKey, "");  // FIXME

  Serial.println("Stored date:");
  Serial.println(stored_date);
  Serial.println("Compiled date:");
  Serial.println(compiled_date);

  Serial.print("First run? ");
  if (!stored_date.equals(compiled_date)) {
    Serial.println("yes");
  } else {
    Serial.println("no");
  }

  preferences.putString(dateKey, compiled_date); // FIXME?
  preferences.end();
  return !stored_date.equals(compiled_date);
}

void ConfigController::checkSetupCompleted()
{
  // check if setup went through after new config - avoid endless boot-loop
  preferences.begin("setup", false);
  if (preferences.getBool("setupComplete", true) == false) {
    Serial.println("Setup not done, resetting config?"); //TODO not working!
    resetPreferences();
  }
  else{
    Serial.println("Setup done, continue.");
  }
  preferences.putBool("setupComplete", false);
  preferences.end();
}
