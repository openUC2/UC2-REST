#include "../../config.h"
#ifdef IS_WIFI
#include "WifiController.h"

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

void WifiController::handelMessages()
{
  wifi.server->handleClient();
}

void WifiController::initWifiAP(String ssid)
{
  Serial.print(F("Network SSID (AP): "));
  Serial.println(ssid);

  WiFi.softAP(ssid.c_str());
  Serial.print(F("AP IP address: "));
  Serial.println(WiFi.softAPIP());
}

void WifiController::joinWifi(String ssid, String password)
{
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  int nConnectTrials = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    nConnectTrials += 1;
    if (nConnectTrials > 10)
      ESP.restart();
    // we can even make the ESP32 to sleep
  }

  Serial.print(F("Connected. IP: "));
  Serial.println(WiFi.localIP());
}

void WifiController::autoconnectWifi(boolean isResetWifiSettings)
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  if (isResetWifiSettings)
  {
    Serial.println(F("First run => resetting Wifi Settings"));
    wm->resetSettings();
  }
  wm->setHostname(hostname);
  wm->setConfigPortalBlocking(false);
  // wm.setConfigPortalBlocking(false);
  // wm.setConfigPortalTimeout(90); // auto close configportal after n seconds
  wm->setConnectTimeout(10);

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result
  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm->autoConnect(mSSIDAP.c_str()); // password protected ap

  if (!res)
  {
    Serial.println(F("Failed to connect, start access point"));
    WiFi.mode(WIFI_AP);
    initWifiAP(mSSIDAP);
  }
  else
  {
    // if you get here you have connected to the WiFi
    Serial.println(F("connected..."));
  }

  Serial.print(F("Connected. IP: "));
  Serial.println(WiFi.localIP());
}

void WifiController::startserver()
{
  wifi.server->begin();
}

void WifiController::setup_routing()
{
  Serial.println("Setting up HTTP Routing");
  wifi.server->on(state_act_endpoint, HTTP_POST, RestApi::State_act);
  wifi.server->on(state_get_endpoint, HTTP_POST, RestApi::State_get);
  wifi.server->on(state_set_endpoint, HTTP_POST, RestApi::State_set);

  wifi.server->on(identity_endpoint, RestApi::getIdentity);

  server->on(ota_endpoint, HTTP_GET, RestApi::ota);
  /*handling uploading firmware file */
  server->on(update_endpoint, HTTP_POST, RestApi::update, RestApi::upload);

  server->on(features_endpoint,HTTP_GET, RestApi::getEndpoints);

  // POST
#ifdef IS_MOTOR
  wifi.server->on(motor_act_endpoint, HTTP_POST, RestApi::FocusMotor_act);
  wifi.server->on(motor_get_endpoint, HTTP_POST, RestApi::FocusMotor_get);
  wifi.server->on(motor_set_endpoint, HTTP_POST, RestApi::FocusMotor_set);
#endif

#ifdef IS_DAC
  wifi.server->on(dac_act_endpoint, HTTP_POST, RestApi::Dac_act);
  wifi.server->on(dac_get_endpoint, HTTP_POST, RestApi::Dac_get);
  wifi.server->on(dac_set_endpoint, HTTP_POST, RestApi::Dac_set);
#endif

#ifdef IS_LASER
  wifi.server->on(laser_act_endpoint, HTTP_POST, RestApi::Laser_act);
  wifi.server->on(laser_get_endpoint, HTTP_POST, RestApi::Laser_get);
  wifi.server->on(laser_set_endpoint, HTTP_POST, RestApi::Laser_set);
#endif

#ifdef IS_ANALOG
  wifi.server->on(analog_act_endpoint, HTTP_POST, RestApi::Analog_act);
  wifi.server->on(analog_get_endpoint, HTTP_POST, RestApi::Analog_get);
  wifi.server->on(analog_set_endpoint, HTTP_POST, RestApi::Analog_set);
#endif

#ifdef IS_DIGITAL
  wifi.server->on(digital_act_endpoint, HTTP_POST, RestApi::Digital_act);
  wifi.server->on(digital_get_endpoint, HTTP_POST, RestApi::Digital_get);
  wifi.server->on(digital_set_endpoint, HTTP_POST, RestApi::Digital_set);
#endif

#ifdef IS_PID
  wifi.server->on(PID_act_endpoint, HTTP_POST, RestApi::Pid_act);
  wifi.server->on(PID_get_endpoint, HTTP_POST, RestApi::Pid_get);
  wifi.server->on(PID_set_endpoint, HTTP_POST, RestApi::Pid_set);
#endif

#ifdef IS_LED
  wifi.server->on(ledarr_act_endpoint, HTTP_POST, RestApi::Led_act);
  wifi.server->on(ledarr_get_endpoint, HTTP_POST, RestApi::Led_get);
  wifi.server->on(ledarr_set_endpoint, HTTP_POST, RestApi::Led_set);
#endif

#ifdef IS_SLM
  wifi.server->on(slm_act_endpoint, HTTP_POST, RestApi::Slm_act);
  wifi.server->on(slm_get_endpoint, HTTP_POST, RestApi::Slm_get);
  wifi.server->on(slm_set_endpoint, HTTP_POST, RestApi::Slm_set);
#endif

  wifi.server->on(config_act_endpoint, HTTP_POST, RestApi::Config_act);
  wifi.server->on(config_get_endpoint, HTTP_POST, RestApi::Config_get);
  wifi.server->on(config_set_endpoint, HTTP_POST, RestApi::Config_set);
}

#endif
