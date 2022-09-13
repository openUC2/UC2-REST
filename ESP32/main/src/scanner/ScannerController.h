
#ifdef IS_SCANNER
#pragma once
#include "../../config.h"
#include <ArduinoJson.h>
#include "../../pinstruct.h"
//#include <FreeRTOS.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#ifdef IS_LASER
    #include "../laser/LaserController.h"
#endif


class ScannerController
{
private:
    
    /* data */
public:
    ScannerController();
    ~ScannerController();

    bool DEBUG = false;

    PINDEF * pins;
    bool isScanRunning = false;
    int  scannerPinX = 25;
    int  scannerPinY = 26;
    int  scannerPinLaser = 4;

    int  scannerXFrameMax= 5;
    int  scannerXFrameMin= 0;
    int  scannerYFrameMax= 5;
    int  scannerYFrameMin= 0;
    int scannerXStep = 5;
    int scannerYStep = 5;

    int  scannerxMin = 0;
    int  scanneryMin = 0;
    int  scannerxMax = 255;
    int  scanneryMax = 255;
    int  scannertDelay = 0;
    int  scannerEnable = 0;
    int  scannerExposure = 0;
    int  scannerLaserVal = 255;
    int scannerDelay = 0;

    int scannernFrames = 1;

    void act(DynamicJsonDocument * jsonDocument);
    void get(DynamicJsonDocument * jsonDocument);
    void set(DynamicJsonDocument * jsonDocument);
    void setup(PINDEF * pins);
    void background();
    static void controlGalvoTask(void * parameters);
};
static ScannerController scanner;
#endif