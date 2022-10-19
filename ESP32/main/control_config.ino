

void config_act_fct() {

  if (jsonDocument.containsKey("resetConfig")) {
    if (DEBUG) Serial.println("resetConfig");
    resetConfigurations();
  }
  if (jsonDocument.containsKey("applyConfig")) {
    if (DEBUG) Serial.println("applyConfig");
    if (DEBUG) Serial.println("Restarting..");
    ESP.restart();
  }
  if (jsonDocument.containsKey("printConfig")) {
    if (DEBUG) Serial.println("printConfig");
    printConfig();
  }

  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void config_set_fct() {
  setConfigurations();
}

void config_get_fct() {
  jsonDocument.clear();
  loadConfiguration();
}

bool resetConfigurations() {
  Serial.println("Resetting Hardware configuration!!!");
  preferences.begin(prefNamespace , false);
  preferences.clear();
  preferences.end();
  return true;
}

bool setConfigurations() {
  /*
    This function sets the preferences for the parameters based on a JSON document.
  */
  if (DEBUG) Serial.println("Setting Hardware Configuration");

  preferences.begin(prefNamespace , false);

  // MOTOR X
  if (jsonDocument.containsKey(keyMotXStepPin))
    preferences.putUInt(keyMotXStepPin, jsonDocument[keyMotXStepPin]);
  if (jsonDocument.containsKey(keyMotXDirPin))
    preferences.putUInt(keyMotXDirPin, jsonDocument[keyMotXDirPin]);

  // MOTOR Y
  if (jsonDocument.containsKey(keyMotYStepPin))
    preferences.putUInt(keyMotYStepPin, jsonDocument[keyMotYStepPin]);
  if (jsonDocument.containsKey(keyMotYDirPin))
    preferences.putUInt(keyMotYDirPin, jsonDocument[keyMotYDirPin]);

  // MOTOR Z
  if (jsonDocument.containsKey(keyMotZStepPin))
    preferences.putUInt(keyMotZStepPin, jsonDocument[keyMotZStepPin]);
  if (jsonDocument.containsKey(keyMotZDirPin))
    preferences.putUInt(keyMotZDirPin, jsonDocument[keyMotZDirPin]);

  // MOTOR A
  if (jsonDocument.containsKey(keyMotAStepPin))
    preferences.putUInt(keyMotAStepPin, jsonDocument[keyMotAStepPin]);
  if (jsonDocument.containsKey(keyMotADirPin))
    preferences.putUInt(keyMotADirPin, jsonDocument[keyMotADirPin]);

  // MOTOR ENABLE
  if (jsonDocument.containsKey(keyMotEnable))
    preferences.putUInt(keyMotEnable, jsonDocument[keyMotEnable]);
 
  // LEDARRAY
  if (jsonDocument.containsKey(keyLEDArray))
    preferences.putUInt(keyLEDArray, jsonDocument[keyLEDArray]);
  if (jsonDocument.containsKey(keyLEDNumLEDArray))
    preferences.putUInt(keyLEDNumLEDArray, jsonDocument[keyLEDNumLEDArray]);

  // DIGITAL PINN
  if (jsonDocument.containsKey(keyDigital1Pin))
    preferences.putUInt(keyDigital1Pin, jsonDocument[keyDigital1Pin]);
  if (jsonDocument.containsKey(keyDigital2Pin))
    preferences.putUInt(keyDigital2Pin, jsonDocument[keyDigital2Pin]);

  // ANALOG PIN
  if (jsonDocument.containsKey(keyAnalog1Pin))
    preferences.putUInt(keyAnalog1Pin, jsonDocument[keyAnalog1Pin]);
  if (jsonDocument.containsKey(keyAnalog2Pin))
    preferences.putUInt(keyAnalog2Pin, jsonDocument[keyAnalog2Pin]);
  if (jsonDocument.containsKey(keyAnalog3Pin))
    preferences.putUInt(keyAnalog3Pin, jsonDocument[keyAnalog3Pin]);

  // LASER PIN
  if (jsonDocument.containsKey(keyLaser1Pin))
    preferences.putUInt(keyLaser1Pin, jsonDocument[keyLaser1Pin]);
  if (jsonDocument.containsKey(keyLaser2Pin))
    preferences.putUInt(keyLaser2Pin, jsonDocument[keyLaser2Pin]);
  if (jsonDocument.containsKey(keyLaser3Pin))
    preferences.putUInt(keyLaser3Pin, jsonDocument[keyLaser3Pin]);

  // DAC FAKE PIN
  if (jsonDocument.containsKey(keyDACfake1Pin))
    preferences.putUInt(keyDACfake1Pin, jsonDocument[keyDACfake1Pin]);
  if (jsonDocument.containsKey(keyDACfake2Pin))
    preferences.putUInt(keyDACfake2Pin, jsonDocument[keyDACfake2Pin]);

  // IDENTIFIER
  if (jsonDocument.containsKey(keyIdentifier)) {
    String Identifier = jsonDocument[keyIdentifier];
    preferences.putString(keyIdentifier, Identifier);
  }

  // WIFI
  if (jsonDocument.containsKey(keyWifiSSID)) {
    String WifiSSID = jsonDocument[keyWifiSSID];
    preferences.putString(keyWifiSSID, WifiSSID);
  }
  if (jsonDocument.containsKey(keyWifiPW)) {
    String WifiPW = jsonDocument[keyWifiPW];
    preferences.putString(keyWifiPW, WifiPW);
  }

  // Playstation
  if (jsonDocument.containsKey(keyPS3Mac)) {
    String PS3MacTmp = jsonDocument[keyPS3Mac];
    preferences.putString(keyPS3Mac, PS3MacTmp);
  }
  if (jsonDocument.containsKey(keyPS4Mac)) {
    String PS4MacTmp = jsonDocument[keyPS4Mac];
    preferences.putString(keyPS4Mac, PS4MacTmp);
  }
  preferences.end();


  // indicate that new config is applied
  preferences.begin("setup" , false);
  preferences.putBool("isNewConfig", true);
  preferences.end();
  



  return true;
}


bool loadConfiguration() {
  /* This function gets the preferences for the parameters
    and returns a JSON document. */

  preferences.begin(prefNamespace, false);
  STEP_PIN_X = preferences.getUInt(keyMotXStepPin, STEP_PIN_X);
  DIR_PIN_X = preferences.getUInt(keyMotXDirPin, DIR_PIN_X);

  STEP_PIN_Y = preferences.getUInt(keyMotYStepPin, STEP_PIN_Y);
  DIR_PIN_Y = preferences.getUInt(keyMotYDirPin, DIR_PIN_Y);

  STEP_PIN_Z = preferences.getUInt(keyMotZStepPin, STEP_PIN_Z);
  DIR_PIN_Z = preferences.getUInt(keyMotZDirPin, DIR_PIN_Z);

  STEP_PIN_A = preferences.getUInt(keyMotAStepPin, STEP_PIN_A);
  DIR_PIN_A = preferences.getUInt(keyMotADirPin, DIR_PIN_A);

  ENABLE_PIN = preferences.getUInt(keyMotEnable, ENABLE_PIN);

  LED_ARRAY_PIN = preferences.getUInt(keyLEDArray , LED_ARRAY_PIN);
  LED_ARRAY_NUM = preferences.getUInt(keyLEDNumLEDArray, LED_ARRAY_NUM);

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
  WifiSSID = preferences.getString(keyWifiSSID, WifiSSID).c_str();
  WifiPW = preferences.getString(keyWifiPW, WifiPW).c_str();
  PS3Mac = preferences.getString(keyPS3Mac, PS3Mac).c_str();
  PS4Mac = preferences.getString(keyPS4Mac, PS4Mac).c_str();

  preferences.end();

  // return preferences as json document
  config2json();

  return true;
}

void printConfig() {
  // Send JSON information back
  Serial.println("++");
  config2json();
  serializeJsonPretty(jsonDocument, Serial);
  Serial.println();
  Serial.println("--");
  jsonDocument.clear();
  jsonDocument.garbageCollect();

}

void config2json() {
  // Assign to JSON jsonDocumentument
  jsonDocument.clear();

  jsonDocument[keyMotXStepPin] = STEP_PIN_X;
  jsonDocument[keyMotXDirPin] = DIR_PIN_X;
  jsonDocument[keyMotYStepPin] = STEP_PIN_Y;
  jsonDocument[keyMotYDirPin] = DIR_PIN_Y;
  jsonDocument[keyMotZStepPin] = STEP_PIN_Z;
  jsonDocument[keyMotZDirPin] = DIR_PIN_Z;
  jsonDocument[keyMotAStepPin] = STEP_PIN_A;
  jsonDocument[keyMotADirPin] = DIR_PIN_A;
  jsonDocument[keyMotEnable] = ENABLE_PIN;
  jsonDocument[keyLEDArray] = LED_ARRAY_PIN;
  jsonDocument[keyLEDNumLEDArray] = LED_ARRAY_NUM;
  jsonDocument[keyDigital1Pin] = DIGITAL_PIN_1;
  jsonDocument[keyDigital2Pin] = DIGITAL_PIN_2;
  jsonDocument[keyAnalog1Pin] = ANALOG_PIN_1;
  jsonDocument[keyAnalog2Pin] = ANALOG_PIN_2;
  jsonDocument[keyAnalog3Pin] = ANALOG_PIN_3;
  jsonDocument[keyLaser1Pin] = LASER_PIN_1;
  jsonDocument[keyLaser2Pin] = LASER_PIN_2;
  jsonDocument[keyLaser3Pin] = LASER_PIN_3;
  jsonDocument[keyDACfake1Pin] = DAC_FAKE_PIN_1;
  jsonDocument[keyDACfake2Pin] = DAC_FAKE_PIN_2;
  jsonDocument[keyIdentifier] = IDENTIFIER_NAME;
  jsonDocument[keyWifiSSID] = WifiSSID;
  jsonDocument[keyWifiPW] = WifiPW;
  jsonDocument[keyPS3Mac] = PS3Mac;
  jsonDocument[keyPS4Mac] = PS4Mac;

  if (DEBUG) Serial.println("Current pin definitions:");
  if (DEBUG) serializeJsonPretty(jsonDocument, Serial);

}


/*
   wrapper for HTTP requests
*/
void config_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  config_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void config_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  config_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void config_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  config_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
