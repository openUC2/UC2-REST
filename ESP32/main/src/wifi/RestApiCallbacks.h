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
#ifdef IS_SLM
    #include "../slm/SlmController.h"
#endif
#if defined IS_DAC || defined IS_DAC_FAKE
    #include "../dac/DacController.h"
#endif
namespace RestApi
{
    void handleNotFound();
    void ota();
    void update();
    void upload();
    void deserialize();
    void serialize();
    void getIdentity();
    void getEndpoints();
    void scanWifi();
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