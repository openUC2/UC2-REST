#include "../../config.h"
#ifdef IS_WIFI
#include "WifiController.h"

WifiController::WifiController(/* args */)
{
}
WifiController::~WifiController()
{
}

void WifiController::handelMessages()
{
  if(server != nullptr)
    server->handleClient();
}

void WifiController::createJsonDoc()
{
  jsonDocument = new DynamicJsonDocument(32784);
  Serial.print("WifiController::createJsonDoc is null:");
  Serial.print(jsonDocument == nullptr);
}

void WifiController::createAp(String ssid, String password)
{
  Serial.println("Ssid empty start softap");
    WiFi.disconnect();
    if (ssid == "")
    {
      WiFi.softAP(mSSIDAP.c_str());
    }
    else if (password == "")
    {
      WiFi.softAP(ssid.c_str());
    }
    else
    {
      WiFi.softAP(ssid.c_str(), password.c_str());
    }
    Serial.print(F("Connected. IP: "));
    Serial.println(WiFi.softAPIP());
}

void WifiController::setup(String ssid, String password,bool ap)
{
  if (ssid == "" || ap)
  {
    createAp(ssid,password);
  }
  else
  {
    Serial.print("Connect to:");Serial.print(ssid);
    WiFi.softAPdisconnect(true);
    WiFi.begin(ssid.c_str(), password.c_str());

    int nConnectTrials = 0;
    while (WiFi.status() != WL_CONNECTED <= nConnectTrials)
    {
      Serial.print(".");
      delay(500);
      nConnectTrials += 1;
      // we can even make the ESP32 to sleep
    }
    if (nConnectTrials == 10)
    {
      Serial.print("failed to connect,Start softap");
      createAp(mSSIDAP,"password");
    }
    else
    {
      Serial.print(F("Connected. IP: "));
      Serial.println(WiFi.localIP());
    }
  }
  server = new WebServer(80);
  RestApi::setup(server,jsonDocument);
  setup_routing();
  server->begin();
  Serial.print("HTTP Running server jsondoc nullptr:");
  Serial.print(server == nullptr);
  Serial.print(jsonDocument == nullptr);
}

void WifiController::setup_routing()
{
  Serial.println("Setting up HTTP Routing");
  server->onNotFound(RestApi::handleNotFound);
  server->on(state_act_endpoint, HTTP_POST, RestApi::State_act);
  server->on(state_get_endpoint, HTTP_POST, RestApi::State_get);
  server->on(state_set_endpoint, HTTP_POST, RestApi::State_set);

  server->on(identity_endpoint, RestApi::getIdentity);

  server->on(ota_endpoint, HTTP_GET, RestApi::ota);
  /*handling uploading firmware file */
  server->on(update_endpoint, HTTP_POST, RestApi::update, RestApi::upload);

  server->on("/",HTTP_GET, RestApi::getEndpoints);
  server->on(scanwifi_endpoint, HTTP_GET, RestApi::scanWifi);

  // POST
#ifdef IS_MOTOR
  server->on(motor_act_endpoint, HTTP_POST, RestApi::FocusMotor_act);
  server->on(motor_get_endpoint, HTTP_POST, RestApi::FocusMotor_get);
  server->on(motor_set_endpoint, HTTP_POST, RestApi::FocusMotor_set);
#endif

#ifdef IS_DAC
  server->on(dac_act_endpoint, HTTP_POST, RestApi::Dac_act);
  server->on(dac_get_endpoint, HTTP_POST, RestApi::Dac_get);
  server->on(dac_set_endpoint, HTTP_POST, RestApi::Dac_set);
#endif

#ifdef IS_LASER
  server->on(laser_act_endpoint, HTTP_POST, RestApi::Laser_act);
  server->on(laser_get_endpoint, HTTP_POST, RestApi::Laser_get);
  server->on(laser_set_endpoint, HTTP_POST, RestApi::Laser_set);
#endif

#ifdef IS_ANALOG
  server->on(analog_act_endpoint, HTTP_POST, RestApi::Analog_act);
  server->on(analog_get_endpoint, HTTP_POST, RestApi::Analog_get);
  server->on(analog_set_endpoint, HTTP_POST, RestApi::Analog_set);
#endif

#ifdef IS_DIGITAL
  server->on(digital_act_endpoint, HTTP_POST, RestApi::Digital_act);
  server->on(digital_get_endpoint, HTTP_POST, RestApi::Digital_get);
  server->on(digital_set_endpoint, HTTP_POST, RestApi::Digital_set);
#endif

#ifdef IS_PID
  server->on(PID_act_endpoint, HTTP_POST, RestApi::Pid_act);
  server->on(PID_get_endpoint, HTTP_POST, RestApi::Pid_get);
  server->on(PID_set_endpoint, HTTP_POST, RestApi::Pid_set);
#endif

#ifdef IS_LED
  server->on(ledarr_act_endpoint, HTTP_POST, RestApi::Led_act);
  server->on(ledarr_get_endpoint, HTTP_POST, RestApi::Led_get);
  server->on(ledarr_set_endpoint, HTTP_POST, RestApi::Led_set);
#endif

#ifdef IS_SLM
  server->on(slm_act_endpoint, HTTP_POST, RestApi::Slm_act);
  server->on(slm_get_endpoint, HTTP_POST, RestApi::Slm_get);
  server->on(slm_set_endpoint, HTTP_POST, RestApi::Slm_set);
#endif

  server->on(config_act_endpoint, HTTP_POST, RestApi::Config_act);
  server->on(config_get_endpoint, HTTP_POST, RestApi::Config_get);
  server->on(config_set_endpoint, HTTP_POST, RestApi::Config_set);

  Serial.println("Setting up HTTP Routing END");
}

#endif
