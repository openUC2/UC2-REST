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
#include "../pid/PidController.h"
#include "../sensor/SensorController.h"


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

WifiController * wifi;

void deserialize()
{
  String body = (*wifi->server).arg("plain");
  deserializeJson((*wifi->jsonDocument), body);
}

void serialize()
{
  serializeJson((*wifi->jsonDocument), wifi->output);
  (*wifi->server).send(200, "application/json", wifi->output);
}

void FocusMotor_act()
{
  deserialize();
  motor->act();
  serialize();
}

void FocusMotor_get()
{
  deserialize();
  motor->get();
  serialize();
}

void FocusMotor_set()
{
  deserialize();
  motor->set();
  serialize();
}

void Laser_act()
{
  deserialize();
  laser->act();
  serialize();
}

void Laser_get()
{
  deserialize();
  laser->get();
  serialize();
}

void Laser_set()
{
  deserialize();
  laser->set();
  serialize();
}

void Dac_act()
{
  deserialize();
  dac->act();
  serialize();
}

void Dac_get()
{
  deserialize();
  dac->get();
  serialize();
}

void Dac_set()
{
  deserialize();
  dac->set();
  serialize();
}


void Led_act()
{
  deserialize();
  led->act();
  serialize();
}

void Led_get()
{
  deserialize();
  led->get();
  serialize();
}

void Led_set()
{
  deserialize();
  led->set();
  serialize();
}


void State_act()
{
  deserialize();
  state->act();
  serialize();
}

void State_get()
{
  deserialize();
  state->get();
  serialize();
}

void State_set()
{
  deserialize();
  state->set();
  serialize();
}

void Analog_act()
{
  deserialize();
  analog->act();
  serialize();
}

void Analog_get()
{
  deserialize();
  analog->get();
  serialize();
}

void Analog_set()
{
  deserialize();
  analog->set();
  serialize();
}

void Digital_act()
{
  deserialize();
  digital->act(wifi->jsonDocument);
  serialize();
}

void Digital_get()
{
  deserialize();
  digital->get(wifi->jsonDocument);
  serialize();
}

void Digital_set()
{
  deserialize();
  digital->set(wifi->jsonDocument);
  serialize();
}


void Pid_act()
{
  deserialize();
  pid->act();
  serialize();
}

void Pid_get()
{
  deserialize();
  pid->get();
  serialize();
}

void Pid_set()
{
  deserialize();
  pid->set();
  serialize();
}

#endif