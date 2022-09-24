#include "ConfigController.h"

namespace Config
{
    Preferences preferences;
    PINDEF * pins;

    const char* prefNamespace = "UC2";
    const char * TAG = "Config";

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

void setWifiConfig(String ssid,String pw, bool ap,bool prefopen)
{
  bool open = prefopen;
    if(!prefopen)
      open = preferences.begin(prefNamespace , false);
    ESP_LOGI(TAG,"setWifiConfig ssid: %s, pw: %s, ap:%s prefopen:%s",ssid,pw,boolToChar(ap),boolToChar(open));
    preferences.putString(keyWifiSSID, ssid);
    preferences.putString(keyWifiPW, pw);
    preferences.putInt(keyWifiAP, ap);
    ESP_LOGI(TAG,"setWifiConfig pref ssid: %s pw:%s",preferences.getString(keyWifiSSID),preferences.getString(keyWifiPW));
    if(!prefopen)
      preferences.end();
  
}

void setJsonToPref(const char* key)
{
  if (WifiController::getJDoc()->containsKey(key))
    preferences.putInt(key, (*WifiController::getJDoc())[key]);
}

void setPinsToJson(const char* key, int val)
{
  (*WifiController::getJDoc())[key] = val;
}

void setup(PINDEF * pin) {
  pins = pin;
  // if we boot for the first time => reset the preferences! // TODO: Smart? If not, we may have the problem that a wrong pin will block bootup
  if (isFirstRun())
  {
    ESP_LOGI(TAG,"First Run, resetting config");
  }
  // check if setup went through after new config - avoid endless boot-loop
  //checkSetupCompleted();
}

bool isFirstRun() {
  bool rdystate = preferences.begin(prefNamespace, false);
  ESP_LOGI(TAG,"isFirstRun Start preferences rdy %s", rdystate ? "true" : "false");
  // define preference name
  const char * dateKey = "date";
  const char *compiled_date = __DATE__ " " __TIME__;
  String stored_date = preferences.getString(dateKey, "");  // FIXME

  ESP_LOGI(TAG,"Stored date: %s",stored_date.c_str());
  ESP_LOGI(TAG,"Compiled date: %s", compiled_date);

  ESP_LOGI(TAG,"First run? ");
  if (!stored_date.equals(compiled_date)) {
    ESP_LOGI(TAG,"yes, resetSettings");
    resetPreferences();
    initempty();
    savePreferencesFromPins();
    applyPreferencesToPins();
    preferences.putString(dateKey, compiled_date); // FIXME?
  } else {
    ESP_LOGI(TAG,"no, loadSettings");
    applyPreferencesToPins();
  }
  preferences.end();

  rdystate = preferences.begin(prefNamespace, false);
  ESP_LOGI(TAG,"datatest pref rdy %s", rdystate ? "true" : "false");
  String datetest = preferences.getString(dateKey, "");
  preferences.end();
  ESP_LOGI(TAG,"isFirstRun End datetest:%s",datetest);
  return !stored_date.equals(compiled_date);
}

bool resetPreferences() {
  ESP_LOGI(TAG,"resetPreferences");
  preferences.clear();
  return true;
}

void savePreferencesFromPins()
{
  preferences.putInt(keyAnalog1Pin,(*pins).analog_PIN_1);
  preferences.putInt(keyAnalog1Pin,pins->analog_PIN_1);
  preferences.putInt(keyAnalog2Pin,pins->analog_PIN_2);
  preferences.putInt(keyAnalog3Pin,pins->analog_PIN_3);

  preferences.putInt(keyMotorAStepPin,pins->STEP_A);
  preferences.putInt(keyMotorXStepPin,pins->STEP_X);
  preferences.putInt(keyMotorYStepPin,pins->STEP_Y);
  preferences.putInt(keyMotorZStepPin,pins->STEP_Z);

  preferences.putInt(keyMotorADirPin,pins->DIR_A);
  preferences.putInt(keyMotorXDirPin,pins->DIR_X);
  preferences.putInt(keyMotorYDirPin,pins->DIR_Y);
  preferences.putInt(keyMotorZDirPin,pins->DIR_Z);

  preferences.putInt(keyMotorEnable, pins->ENABLE);

  preferences.putInt(keyLEDArray,pins->LED_ARRAY_PIN);
  preferences.putInt(keyLEDNumLEDArray,pins->LED_ARRAY_NUM);


  preferences.putInt(keyDigital1Pin,pins->digital_PIN_1);
  preferences.putInt(keyDigital2Pin,pins->digital_PIN_2);
  preferences.putInt(keyDigital3Pin,pins->digital_PIN_3);

  preferences.putInt(keyAnalog1Pin,pins->analog_PIN_1);
  preferences.putInt(keyAnalog2Pin,pins->analog_PIN_2);
  preferences.putInt(keyAnalog3Pin,pins->analog_PIN_3);

  preferences.putInt(keyLaser1Pin,pins->LASER_PIN_1);
  preferences.putInt(keyLaser2Pin,pins->LASER_PIN_2);
  preferences.putInt(keyLaser3Pin,pins->LASER_PIN_3);

  preferences.putInt(keyDACfake1Pin,pins->dac_fake_1);
  preferences.putInt(keyDACfake2Pin,pins->dac_fake_2);
  preferences.putString(keyIdentifier, pins->identifier_setup);
  preferences.putString(keyWifiSSID,WifiController::getSsid());
  preferences.putString(keyWifiPW, WifiController::getPw());
  preferences.putInt(keyWifiAP, WifiController::getAp());
}

void initempty()
{
  preferences.putInt(keyAnalog1Pin,0);
  preferences.putInt(keyAnalog1Pin,0);
  preferences.putInt(keyAnalog2Pin,0);
  preferences.putInt(keyAnalog3Pin,0);

  preferences.putInt(keyMotorAStepPin,0);
  preferences.putInt(keyMotorXStepPin,0);
  preferences.putInt(keyMotorYStepPin,0);
  preferences.putInt(keyMotorZStepPin,0);

  preferences.putInt(keyMotorADirPin,0);
  preferences.putInt(keyMotorXDirPin,0);
  preferences.putInt(keyMotorYDirPin,0);
  preferences.putInt(keyMotorZDirPin,0);

  preferences.putInt(keyMotorEnable, 0);

  preferences.putInt(keyLEDArray,0);
  preferences.putInt(keyLEDNumLEDArray,0);


  preferences.putInt(keyDigital1Pin,0);
  preferences.putInt(keyDigital2Pin,0);
  preferences.putInt(keyDigital3Pin,0);

  preferences.putInt(keyAnalog1Pin,0);
  preferences.putInt(keyAnalog2Pin,0);
  preferences.putInt(keyAnalog3Pin,0);

  preferences.putInt(keyLaser1Pin,0);
  preferences.putInt(keyLaser2Pin,0);
  preferences.putInt(keyLaser3Pin,0);

  preferences.putInt(keyDACfake1Pin,0);
  preferences.putInt(keyDACfake2Pin,0);
  preferences.putString(keyIdentifier, "");
  preferences.putString(keyWifiSSID,"Uc2");
  preferences.putString(keyWifiPW, "");
  preferences.putInt(keyWifiAP, true);
}

bool setPreferences() {
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
  setJsonToPref(keyDigital3Pin);

  setJsonToPref(keyAnalog1Pin);
  setJsonToPref(keyAnalog2Pin);
  setJsonToPref(keyAnalog3Pin);

  setJsonToPref(keyLaser1Pin);
  setJsonToPref(keyLaser2Pin);
  setJsonToPref(keyLaser3Pin);

  setJsonToPref(keyDACfake1Pin);
  setJsonToPref(keyDACfake2Pin);

  preferences.putString(keyIdentifier,  (const char *)(*WifiController::getJDoc())[keyIdentifier]);
  setWifiConfig((*WifiController::getJDoc())[keyWifiSSID],(*WifiController::getJDoc())[keyWifiPW],(*WifiController::getJDoc())[keyWifiAP],true);
  preferences.end();
  return true;
}

void applyPreferencesToPins()
{
  pins->STEP_X = preferences.getInt(keyMotorXStepPin);
  pins->DIR_X = preferences.getInt(keyMotorXStepPin);

  pins->STEP_Y = preferences.getInt(keyMotorXStepPin);
  pins->DIR_Y = preferences.getInt(keyMotorYDirPin);

  pins->STEP_Z = preferences.getInt(keyMotorZStepPin);
  pins->DIR_Z = preferences.getInt(keyMotorZDirPin);

  pins->STEP_A = preferences.getInt(keyMotorAStepPin);
  pins->DIR_A = preferences.getInt(keyMotorADirPin);

  pins->ENABLE = preferences.getInt(keyMotorEnable);

  pins->LED_ARRAY_PIN = preferences.getInt(keyLEDArray, pins->LED_ARRAY_PIN);
  pins->LED_ARRAY_NUM = preferences.getInt(keyLEDNumLEDArray, pins->LED_ARRAY_NUM);

  pins->digital_PIN_1 = preferences.getInt(keyDigital1Pin, pins->digital_PIN_1);
  pins->digital_PIN_2 = preferences.getInt(keyDigital2Pin, pins->digital_PIN_2);
  pins->digital_PIN_3 = preferences.getInt(keyDigital3Pin, pins->digital_PIN_3);

  pins->analog_PIN_1 = preferences.getInt(keyAnalog1Pin, pins->analog_PIN_1);
  pins->analog_PIN_2 = preferences.getInt(keyAnalog2Pin, pins->analog_PIN_2);
  pins->analog_PIN_3 = preferences.getInt(keyAnalog3Pin, pins->analog_PIN_3);

  pins->LASER_PIN_1 = preferences.getInt(keyLaser1Pin, pins->LASER_PIN_1);
  pins->LASER_PIN_2 = preferences.getInt(keyLaser2Pin, pins->LASER_PIN_2);
  pins->LASER_PIN_3 = preferences.getInt(keyLaser3Pin, pins->LASER_PIN_3);

  pins->dac_fake_1 = preferences.getInt(keyDACfake1Pin, pins->dac_fake_1);
  pins->dac_fake_2 = preferences.getInt(keyDACfake2Pin, pins->dac_fake_2);

  
  pins->identifier_setup = preferences.getString(keyIdentifier, pins->identifier_setup).c_str();
  if(WifiController::getSsid() != nullptr)
    ESP_LOGI(TAG,"ssid bevor:%s", WifiController::getSsid());
  else
    ESP_LOGI(TAG,"ssid is nullptr");
  String ssid = preferences.getString(keyWifiSSID);
  
  String pw = preferences.getString(keyWifiPW);
  bool ap = preferences.getInt(keyWifiAP);
  WifiController::setWifiConfig(ssid,pw,ap);
  ESP_LOGI(TAG,"ssid after:%s pref ssid:%s", WifiController::getSsid().c_str(), preferences.getString(keyWifiSSID, WifiController::getSsid()).c_str());
}

bool getPreferences() {

  preferences.begin(prefNamespace, false);
  applyPreferencesToPins();

  WifiController::getJDoc()->clear();
  
  // Assign to JSON WifiController::getJDoc()ument
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

  (*WifiController::getJDoc())[keyIdentifier] = pins->identifier_setup;
  (*WifiController::getJDoc())[keyWifiSSID] = WifiController::getSsid();
  (*WifiController::getJDoc())[keyWifiPW] = WifiController::getSsid();

  serializeJson(*WifiController::getJDoc(), Serial);

  preferences.end();
  return true;

}

void loop() {
  if (Serial.available()) {
    DeserializationError error = deserializeJson((*WifiController::getJDoc()), Serial);

    if (error) {
      ESP_LOGI(TAG,"%s",error);
    }
    else {
      setPreferences();
      getPreferences();
    }
  }
}

void act()
{
    resetPreferences();
    WifiController::getJDoc()->clear();
    (*WifiController::getJDoc())["return"] = 1;
}

void set()
{
    setPreferences();
    WifiController::getJDoc()->clear();
    (*WifiController::getJDoc())["return"] = 1;
}

void get()
{
    getPreferences();
    WifiController::getJDoc()->clear();
    (*WifiController::getJDoc())["return"] = 1;
}



}
