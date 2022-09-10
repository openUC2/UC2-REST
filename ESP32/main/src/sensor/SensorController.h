#ifndef SensorController_h
#define SensorController_h

#include <ArduinoJson.h>
#include "../../pinstruct.h"
#ifdef IS_LASER
#include "../laser/LaserController.h"
#endif

class SensorController
{
private:
    /* data */
public:
    SensorController(/* args */);
    ~SensorController();
    bool DEBUG = false;
    DynamicJsonDocument * jsonDocument;
    PINDEF * pins;

    int N_sensor_avg; //no idea if it should be equal to that that one inside PidController.h 

    void setup(PINDEF * pins,DynamicJsonDocument * jsonDocument);
    void act();
    void set();
    void get();
};

static SensorController sensor;

#endif