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

static const char* state_act_endpoint = "/state_act";
static const char* state_set_endpoint = "/state_set";
static const char* state_get_endpoint = "/state_get";

static const char* laser_act_endpoint = "/laser_act";
static const char* laser_set_endpoint = "/laser_set";
static const char* laser_get_endpoint = "/laser_get";

static const char* motor_act_endpoint = "/motor_act";
static const char* motor_set_endpoint = "/motor_set";
static const char* motor_get_endpoint = "/motor_get";

#ifdef IS_DAC
static const char* dac_act_endpoint = "/dac_act";
static const char* dac_set_endpoint = "/dac_set";
static const char* dac_get_endpoint = "/dac_get";
#endif

#ifdef IS_ANALOG
static const char* analog_act_endpoint = "/analog_act";
static const char* analog_set_endpoint = "/analog_set";
static const char* analog_get_endpoint = "/analog_get";
#endif

#ifdef IS_DIGITAL
static const char* digital_act_endpoint = "/digital_act";
static const char* digital_set_endpoint = "/digital_set";
static const char* digital_get_endpoint = "/digital_get";
#endif

static const char* ledarr_act_endpoint = "/ledarr_act";
static const char* ledarr_set_endpoint = "/ledarr_set";
static const char* ledarr_get_endpoint = "/ledarr_get";

#ifdef IS_SLM
static const char* slm_act_endpoint = "/slm_act";
static const char* slm_set_endpoint = "/slm_set";
static const char* slm_get_endpoint = "/slm_get";
#endif

#ifdef IS_READSENSOR
static const char* readsensor_act_endpoint = "/readsensor_act";
static const char* readsensor_set_endpoint = "/readsensor_set";
static const char* readsensor_get_endpoint = "/readsensor_get";
#endif

#ifdef IS_PID
static const char* PID_act_endpoint = "/PID_act";
static const char* PID_set_endpoint = "/PID_set";
static const char* PID_get_endpoint = "/PID_get";
#endif

class WifiController
{
private:
    /* data */
public:
    WifiController(/* args */)
    {
    wm = new WiFiManager();
    server = new WebServer(80);
    };
    ~WifiController(){
    wm = nullptr;
    server->close();
    server = nullptr;
    };

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

static WifiController * wifi;

static void deserialize()
{
  String body = (*wifi->server).arg("plain");
  deserializeJson((*wifi->jsonDocument), body);
}

static void serialize()
{
  serializeJson((*wifi->jsonDocument), wifi->output);
  (*wifi->server).send(200, "application/json", wifi->output);
}

static void FocusMotor_act()
{
  deserialize();
  motor->act();
  serialize();
}

static void FocusMotor_get()
{
  deserialize();
  motor->get();
  serialize();
}

static void FocusMotor_set()
{
  deserialize();
  motor->set();
  serialize();
}

static void Laser_act()
{
  deserialize();
  laser->act();
  serialize();
}

static void Laser_get()
{
  deserialize();
  laser->get();
  serialize();
}

static void Laser_set()
{
  deserialize();
  laser->set();
  serialize();
}

static void Dac_act()
{
  deserialize();
  dac->act();
  serialize();
}

static void Dac_get()
{
  deserialize();
  dac->get();
  serialize();
}

static void Dac_set()
{
  deserialize();
  dac->set();
  serialize();
}


static void Led_act()
{
  deserialize();
  led->act();
  serialize();
}

static void Led_get()
{
  deserialize();
  led->get();
  serialize();
}

static void Led_set()
{
  deserialize();
  led->set();
  serialize();
}


static void State_act()
{
  deserialize();
  state->act();
  serialize();
}

static void State_get()
{
  deserialize();
  state->get();
  serialize();
}

static void State_set()
{
  deserialize();
  state->set();
  serialize();
}

static void Analog_act()
{
  deserialize();
  analog->act();
  serialize();
}

static void Analog_get()
{
  deserialize();
  analog->get();
  serialize();
}

static void Analog_set()
{
  deserialize();
  analog->set();
  serialize();
}

static void Digital_act()
{
  deserialize();
  digital->act(wifi->jsonDocument);
  serialize();
}

static void Digital_get()
{
  deserialize();
  digital->get(wifi->jsonDocument);
  serialize();
}

static void Digital_set()
{
  deserialize();
  digital->set(wifi->jsonDocument);
  serialize();
}


static void Pid_act()
{
  deserialize();
  pid->act();
  serialize();
}

static void Pid_get()
{
  deserialize();
  pid->get();
  serialize();
}

static void Pid_set()
{
  deserialize();
  pid->set();
  serialize();
}

#endif