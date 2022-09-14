#include "../../config.h"
#ifdef IS_WIFI
#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include "parameters_wifi.h"
#include "../state/State.h"
#include "Endpoints.h"
#include "RestApiCallbacks.h"

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
#include "../led/led_controller.h"
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

class WifiController
{
private:
    const String mSSIDAP = F("UC2");
    const String hostname = F("youseetoo");

    /* data */
public:
    WifiController(/* args */);
    ~WifiController();
    
    DynamicJsonDocument * jsonDocument;
    WiFiManager * wm;
    WebServer * server;
    
    char output[1000];

    void setup_routing();
    void handelMessages();
    void initWifiAP(String ssid);
    void joinWifi(String ssid, String password);
    void autoconnectWifi(boolean isResetWifiSettings);
    void startserver();
    


};
static WifiController wifi;
#endif
