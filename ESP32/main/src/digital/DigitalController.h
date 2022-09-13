#pragma once
#include "../../config.h"
#include <ArduinoJson.h>
#include "../../pinstruct.h"

class DigitalController
{
private:
    /* data */
public:
    DigitalController(/* args */);
    ~DigitalController();

    bool isBusy;
    bool DEBUG = false;
    PINDEF * pins;

    int digital_val_1 = 0;
    int digital_val_2 = 0;
    int digital_val_3 = 0;

    void act(DynamicJsonDocument * jsonDocument);
    void set(DynamicJsonDocument * jsonDocument);
    void get(DynamicJsonDocument * jsonDocument);
    void setup(PINDEF * pins);
};
static DigitalController digital;
