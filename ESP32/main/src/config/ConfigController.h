#ifndef ConfigController_h
#define ConfigController_h

#include <Preferences.h>
#include <ArduinoJson.h>
#include "JsonKeys.h"
#include "../../pinstruct.h"

class ConfigController
{
private:
    Preferences preferences;
    /* data */
public:
    ConfigController(/* args */);
    ~ConfigController();

    String identifier_setup = "pindef_multicolour_wemos_lena";
    String wifiSSID = "SSID";
    String wifiPW = "PW";
    PINDEF * pins;
    DynamicJsonDocument * jsonDocument;

    void setup();
    bool resetPreferences();
    bool setPreferences();
    bool getPreferences();
    void loop();

};




#endif