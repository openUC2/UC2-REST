#ifndef WifiController_h
#define WifiController_h

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include "parameters_wifi.h"
#include "../state/State.h"
#include "../motor/FocusMotor.h"
#include "../laser/LaserController.h"
#include "../dac/DacController.h"
#include "../led/led_controller.h"
#include "../analog/AnalogController.h"
#include "../digital/DigitalController.h"


    /*
   Register functions
*/

const char* state_act_endpoint = "/state_act";
const char* state_set_endpoint = "/state_set";
const char* state_get_endpoint = "/state_get";

const char* laser_act_endpoint = "/laser_act";
const char* laser_set_endpoint = "/laser_set";
const char* laser_get_endpoint = "/laser_get";

const char* motor_act_endpoint = "/motor_act";
const char* motor_set_endpoint = "/motor_set";
const char* motor_get_endpoint = "/motor_get";

#ifdef IS_DAC
const char* dac_act_endpoint = "/dac_act";
const char* dac_set_endpoint = "/dac_set";
const char* dac_get_endpoint = "/dac_get";
#endif

#ifdef IS_ANALOG
const char* analog_act_endpoint = "/analog_act";
const char* analog_set_endpoint = "/analog_set";
const char* analog_get_endpoint = "/analog_get";
#endif

#ifdef IS_DIGITAL
const char* digital_act_endpoint = "/digital_act";
const char* digital_set_endpoint = "/digital_set";
const char* digital_get_endpoint = "/digital_get";
#endif

const char* ledarr_act_endpoint = "/ledarr_act";
const char* ledarr_set_endpoint = "/ledarr_set";
const char* ledarr_get_endpoint = "/ledarr_get";

#ifdef IS_SLM
const char* slm_act_endpoint = "/slm_act";
const char* slm_set_endpoint = "/slm_set";
const char* slm_get_endpoint = "/slm_get";
#endif

#ifdef IS_READSENSOR
const char* readsensor_act_endpoint = "/readsensor_act";
const char* readsensor_set_endpoint = "/readsensor_set";
const char* readsensor_get_endpoint = "/readsensor_get";
#endif

#ifdef IS_PID
const char* PID_act_endpoint = "/PID_act";
const char* PID_set_endpoint = "/PID_set";
const char* PID_get_endpoint = "/PID_get";
#endif

class WifiController
{
private:
    /* data */
public:
    WifiController(/* args */);
    ~WifiController();

    DynamicJsonDocument * jsonDocument;
    WiFiManager * wm;
    WebServer * server;
    State * state;
    FocusMotor * motor;
    LaserController * laser;
    DacController * dac;
    led_controller * led;
    AnalogController * analog;
    DigitalController * digital;
    
    
    
    char output[1000];

    void setup_routing();
    void handelMessages();
    void init_Spiffs();
    void initWifiAP(const char *ssid);
    void joinWifi(const char *ssid, const char *password);
    void autoconnectWifi(boolean isResetWifiSettings);
    void startserver();
    
};

WifiController::WifiController(/* args */)
{
    wm = new WiFiManager();
    server = new WebServer(80);
}

WifiController::~WifiController()
{
    wm = nullptr;
    server->close();
    server = nullptr;
}

WifiController * globalWifi;

void deserialize()
{
  String body = (*globalWifi->server).arg("plain");
  deserializeJson((*globalWifi->jsonDocument), body);
}

void serialize()
{
  serializeJson((*globalWifi->jsonDocument), globalWifi->output);
  (*globalWifi->server).send(200, "application/json", globalWifi->output);
}

void FocusMotor_motor_act_fct_http_wrapper()
{
  globalWifi->motor->motor_act_fct_http();
}

void FocusMotor_motor_get_fct_http_wrapper()
{
  globalWifi->motor->motor_get_fct_http();
}

void FocusMotor_motor_set_fct_http_wrapper()
{
  globalWifi->motor->motor_set_fct_http();
}

void Laser_act_fct_http_wrapper()
{
  globalWifi->laser->LASER_act_fct();
}

void Laser_get_fct_http_wrapper()
{
  globalWifi->laser->LASER_get_fct();
}

void Laser_set_fct_http_wrapper()
{
  globalWifi->laser->LASER_set_fct();
}

void Dac_act_fct_http_wrapper()
{
  globalWifi->dac->dac_act_fct();
}

void Dac_get_fct_http_wrapper()
{
  globalWifi->dac->dac_get_fct();
}

void Dac_set_fct_http_wrapper()
{
  globalWifi->dac->dac_set_fct();
}


void Led_act_fct_http_wrapper()
{
  globalWifi->led->ledarr_act_fct();
}

void Led_get_fct_http_wrapper()
{
  globalWifi->led->ledarr_get_fct();
}

void Led_set_fct_http_wrapper()
{
  globalWifi->led->ledarr_set_fct();
}


void State_act_fct_http_wrapper()
{
  globalWifi->state->state_act_fct_http();
}

void State_get_fct_http_wrapper()
{
  globalWifi->state->state_get_fct_http();
}

void State_set_fct_http_wrapper()
{
  globalWifi->state->state_set_fct_http();
}

void Analog_act_fct_http_wrapper()
{
  globalWifi->analog->analog_act_fct();
}

void Analog_get_fct_http_wrapper()
{
  globalWifi->analog->analog_get_fct();
}

void Analog_set_fct_http_wrapper()
{
  globalWifi->analog->analog_set_fct();
}

void Digital_act_fct_http_wrapper()
{
  deserialize();
  int digitalid = (*globalWifi->jsonDocument)["digitalid"];
  int digitalval = (*globalWifi->jsonDocument)["digitalval"];
  globalWifi->digital->digital_act_fct(digitalid,digitalval);
  globalWifi->jsonDocument->clear();
  (*globalWifi->jsonDocument)["return"] = 1;
  serialize();
  
}

void Digital_get_fct_http_wrapper()
{
  deserialize();
  globalWifi->digital->digital_get_fct(globalWifi->jsonDocument);
  serialize();
}

void Digital_set_fct_http_wrapper()
{
  deserialize();
  int digitalid = (*globalWifi->jsonDocument)["digitalid"];
  int digitalpin = (*globalWifi->jsonDocument)["digitalpin"];
  globalWifi->digital->digital_set_fct(digitalid,digitalpin);
  globalWifi->jsonDocument->clear();
  (*globalWifi->jsonDocument)["return"] = 1;
  serialize();
}

#endif