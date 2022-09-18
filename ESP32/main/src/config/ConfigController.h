#pragma once

#include <Preferences.h>
#include <ArduinoJson.h>
#include "JsonKeys.h"
#include "../../pinstruct.h"
#include "../wifi/WifiController.h"
#include "esp_log.h"

namespace Config
{
    
    void setJsonToPref(const char * key);
    void setPinsToJson(const char * key, int val);
    void setup(PINDEF * pins);
    bool resetPreferences();
    bool setPreferences();
    bool getPreferences();
    void initempty();
    void savePreferencesFromPins();
    void applyPreferencesToPins();
    void loop();
    void act();
    void set();
    void get();
    bool isFirstRun();
    void setWifiConfig(String ssid,String pw, bool ap,bool prefopen);
}
