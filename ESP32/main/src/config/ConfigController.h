#pragma once

#include <Preferences.h>
#include <ArduinoJson.h>
#include "JsonKeys.h"
#include "../../pinstruct.h"

class ConfigController
{
private:
    Preferences preferences;
    DynamicJsonDocument * jsonDocument;
    void setJsonToPref(const char * key);
    void setPrefToPins(const char * key, int* val);
    void setPinsToJson(const char * key, int val);
    /* data */
public:
    ConfigController(/* args */);
    ~ConfigController();

    PINDEF * pins;
    

    void setup(PINDEF * pins, DynamicJsonDocument * jsonDocument);
    bool resetPreferences();
    bool setPreferences();
    bool getPreferences();
    void loop();
    void act();
    void set();
    void get();
    bool isFirstRun();
    void checkSetupCompleted();
};

static ConfigController config;
