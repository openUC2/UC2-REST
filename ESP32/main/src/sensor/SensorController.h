#pragma once
#include "../../config.h"
#include <ArduinoJson.h>
#include "../../pinstruct.h"
#ifdef IS_LASER
#include "../laser/LaserController.h"
#endif
#include "../wifi/WifiController.h"

class SensorController
{
private:
    /* data */
public:
    SensorController(/* args */);
    ~SensorController();
    bool DEBUG = false;
    PINDEF * pins;

    int N_sensor_avg; //no idea if it should be equal to that that one inside PidController.h 

    void setup(PINDEF * pins);
    void act();
    void set();
    void get();
};

static SensorController sensor;
