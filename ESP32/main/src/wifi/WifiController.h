#include "../../config.h"
#ifdef IS_WIFI
#ifndef WifiController_h
#define WifiController_h

#include "config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include "parameters_wifi.h"
#include "../state/State.h"
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


    /*
   Register functions
*/

static const char* state_act_endpoint = "/state_act";
static const char* state_set_endpoint = "/state_set";
static const char* state_get_endpoint = "/state_get";

#ifdef IS_LASER
static const char* laser_act_endpoint = "/laser_act";
static const char* laser_set_endpoint = "/laser_set";
static const char* laser_get_endpoint = "/laser_get";
#endif

#ifdef IS_MOTOR
static const char* motor_act_endpoint = "/motor_act";
static const char* motor_set_endpoint = "/motor_set";
static const char* motor_get_endpoint = "/motor_get";
#endif

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

static const char* config_act_endpoint = "/config_act";
static const char* config_set_endpoint = "/config_set";
static const char* config_get_endpoint = "/config_get";

class WifiController
{
private:

    static void ota();
    static void update();
    static void upload();
    static void deserialize();
    static void serialize();
#ifdef IS_MOTOR
    static void FocusMotor_act();
    static void FocusMotor_get();
    static void FocusMotor_set();
#endif
#ifdef IS_LASER
    static void Laser_act();
    static void Laser_get();
    static void Laser_set();
#endif
#ifdef IS_DAC
    static void Dac_act();
    static void Dac_get();
    static void Dac_set();
#endif
#ifdef IS_LED
    static void Led_act();
    static void Led_get();
    static void Led_set();
#endif
    static void State_act();
    static void State_get();
    static void State_set();
#ifdef IS_ANALOG
    static void Analog_act();
    static void Analog_get();
    static void Analog_set();
#endif
#ifdef IS_DIGITAL
    static void Digital_act();
    static void Digital_get();
    static void Digital_set();
#endif
#ifdef IS_PID
    static void Pid_act();
    static void Pid_get();
    static void Pid_set();
#endif
    static void Config_act();
    static void Config_get();
    static void Config_set();
#ifdef IS_SLM
    static void Slm_act();
    static void Slm_get();
    static void Slm_set();
#endif
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
    static void getIdentity();

    static bool loadFromSPIFFS(String path);
    static void handleNotFound();
    static void handleSwaggerYaml();
    static void handleSwaggerUI();
    static void handlestandalone();
    static void handleswaggerbundle();
    static void handleswaggercss();


};
static WifiController wifi;
#endif


#endif