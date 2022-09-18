#pragma once
#include "../../config.h"
#include <ArduinoJson.h>
#include "../../pinstruct.h"
#ifdef IS_MOTOR
#include "../motor/FocusMotor.h"
#endif
#include "../wifi/WifiController.h"

class PidController
{
private:
    /* data */
    long returnControlValue(float controlTarget, float sensorValue, float Kp, float Ki, float Kd);
public:
    PidController(/* args */);
    ~PidController();
    PINDEF * pins;
    bool DEBUG = false;

    float errorRunSum=0;
    float previousError=0;
    float stepperMaxValue=2500.;
    float PID_Kp = 10;
    float PID_Ki = 0.1;
    float PID_Kd = 0.1;
    float PID_target = 500;
    float PID_updaterate = 200; // ms
    bool PID_active=false;

    int N_sensor_avg = 50;

    void setup(PINDEF * pins);
    void background();
    void act();
    void get();
    void set();
};

static PidController pid;
