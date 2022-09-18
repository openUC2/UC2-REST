#pragma once
#include "../../config.h"
#include "../../pinstruct.h"
#include <ArduinoJson.h>
#include <String.h>
#include "../wifi/WifiController.h"

class LaserController
{
private:
    /* data */
public:
    LaserController(/* args */);
    ~LaserController();

    PINDEF * pins;
    bool isBusy;
    
    int LASER_val_1 = 0;
    int LASER_val_2 = 0;
    int LASER_val_3 = 0;

    // PWM Stuff - ESP only

    /*
    int pwm_resolution = 15; 
    int pwm_frequency = 800000;  //19000; // 12000;
    int pwm_max = (int)pow(2,pwm_resolution);
    */

    int pwm_resolution = 10; //8bit 256, 10bit  1024, 12bit 4096;
    int pwm_frequency =  5000;//19000; //12000
    long pwm_max = (int)pow(2, pwm_resolution);


    int PWM_CHANNEL_LASER_1 = 0;
    int PWM_CHANNEL_LASER_2 = 1;
    int PWM_CHANNEL_LASER_3 = 2;

    // temperature dependent despeckeling?
    int LASER_despeckle_1 = 0;
    int LASER_despeckle_2 = 0;
    int LASER_despeckle_3 = 0;

    int LASER_despeckle_period_1 = 20;
    int LASER_despeckle_period_2 = 20;
    int LASER_despeckle_period_3 = 20;

    bool DEBUG = false;

    void LASER_despeckle(int LASERdespeckle, int LASERid, int LASERperiod);
    void act();
    void set();
    void get();
    void setup(PINDEF * pins);
    void loop();

};
static LaserController laser;
