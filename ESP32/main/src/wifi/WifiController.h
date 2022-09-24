#include "../../config.h"
#ifdef IS_WIFI
#pragma once

#include <WiFi.h>
#include <HardwareSerial.h>
#include "parameters_wifi.h"
#include "../state/State.h"
#include "RestApiCallbacks.h"
#include "esp_log.h"

#ifdef IS_MOTOR
#include "../motor/FocusMotor.h"
#endif 
#ifdef IS_LASER
#include "../laser/LaserController.h"
#endif
#if defined IS_DAC || defined IS_DAC_FAKE
#include "../dac/DacController.h"
#endif
#ifdef IS_LED
#include "../led/LedController.h"
#endif
#ifdef IS_ANALOG
#include "../analog/AnalogController.h"
#endif
#ifdef IS_DIGITAL
#include "../digital/DigitalController.h"
#endif
#ifdef IS_PID
#include "../pid/PidController.h"
#endif
#ifdef IS_READSENSOR
#include "../sensor/SensorController.h"
#endif
#include "../config/ConfigController.h"
#ifdef IS_SLM
    #include "../slm/SlmController.h"
#endif
#if defined IS_DAC || defined IS_DAC_FAKE
    #include "../dac/DacController.h"
#endif

namespace WifiController
{
    
    void createAp(String ssid, String password);
    
    /* data */

    
    
    void setup_routing();
    void handelMessages();
    void createJsonDoc();
    
    //Wifi
    
    void setWifiConfig(String mSSID,String mPWD,bool ap);
    String getSsid();
    String getPw();
    bool getAp();
    void setup();
    DynamicJsonDocument * getJDoc();
    WebServer * getServer();
}
#endif
