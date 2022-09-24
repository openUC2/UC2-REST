#pragma once
#include "../../config.h"
#include <ArduinoJson.h>
#include <WebServer.h>
#include <nvs_flash.h>
#include "Endpoints.h"
#include "parameters_wifi.h"
#include "../config/ConfigController.h"
#include "../wifi/WifiController.h"
#include <esp_log.h>
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
#ifdef IS_SLM
    #include "../slm/SlmController.h"
#endif
#if defined IS_DAC || defined IS_DAC_FAKE
    #include "../dac/DacController.h"
#endif
namespace RestApi
{
    /*
        handle invalide requests with a error message 
    */
    void handleNotFound();
    void ota();
    void update();
    void upload();
    /*
        load the body data from the client request into the jsondoc
    */
    void deserialize();
    /*
        fill the output from the jsondoc and send a response to the client
    */
    void serialize();
    void getIdentity();
    /* 
        returns an array that contains the endpoints
        endpoint:/features_get or /
        input[]
        output
        [
        "/ota",
        "/update",
        "/identity",
        "/config_act",
        "/config_set",
        "/config_get",
        "/state_act",
        "/state_set",
        "/state_get",
        "/wifi/scan",
        "/wifi/connect",
        "/motor_act",
        "/motor_set",
        "/motor_get",
        "/ledarr_act",
        "/ledarr_set",
        "/ledarr_get"
        ]
    */
    void getEndpoints();
    /*
        start a wifiscan and return the results 
        endpoint:/wifi/scan
        input []
        output
        [
            "ssid1",
            "ssid2",
            ....
        ]
    */
    void scanWifi();
    /*
        connect to a wifi network or create ap
        endpoint:/wifi/connect
        input
        [
            "ssid": "networkid"
            "PW" : "password"
            "AP" : 0
        ]
        output[]
    */
    void connectToWifi();
    void resetNvFLash();
#ifdef IS_MOTOR
    void FocusMotor_act();
    void FocusMotor_get();
    void FocusMotor_set();
#endif
#ifdef IS_LASER
    void Laser_act();
    void Laser_get();
    void Laser_set();
#endif
#ifdef IS_DAC
    void Dac_act();
    void Dac_get();
    void Dac_set();
#endif
#ifdef IS_LED
    void Led_act();
    void Led_get();
    void Led_set();
#endif
    void State_act();
    void State_get();
    void State_set();
#ifdef IS_ANALOG
    void Analog_act();
    void Analog_get();
    void Analog_set();
#endif
#ifdef IS_DIGITAL
    void Digital_act();
    void Digital_get();
    void Digital_set();
#endif
#ifdef IS_PID
    void Pid_act();
    void Pid_get();
    void Pid_set();
#endif
    void Config_act();
    void Config_get();
    void Config_set();
#ifdef IS_SLM
    void Slm_act();
    void Slm_get();
    void Slm_set();
#endif

    
}