#include "../../config.h"
#ifdef IS_WIFI
#include "WifiController.h"
namespace WifiController
{

    const String mSSIDAP = F("UC2");
    const String hostname = F("youseetoo");
    const char * TAG = "Wifi";
    String mSSID = "Uc2";
    String mPWD = "";
    bool mAP = true;
    WebServer * server = nullptr;
    DynamicJsonDocument * jsonDocument;

DynamicJsonDocument * getJDoc()
{
  return jsonDocument;
}

String getSsid()
{
  return mSSID;
}
String getPw()
{
  return mPWD;
}
bool getAp()
{
  return mAP;
}

WebServer * getServer()
{
  return server;
}

void handelMessages()
{
  if(server != nullptr)
    server->handleClient();
}

void createJsonDoc()
{
  jsonDocument = new DynamicJsonDocument(16128);
  ESP_LOGI(TAG,"WifiController::createJsonDoc is null:%s", boolToChar(jsonDocument == nullptr));
}

void setWifiConfig(String SSID,String PWD,bool ap)
{
    ESP_LOGI(TAG,"mssid:%s pw:%s ap:%s", mSSID,mPWD,boolToChar(ap));
    mSSID = SSID;
    mPWD = PWD;
    mAP = ap;
}

void createAp(String ssid, String password)
{
    WiFi.disconnect();
    ESP_LOGI(TAG,"Ssid %s pw %s",ssid,password);
    
    if (ssid.isEmpty())
    {
      ESP_LOGI(TAG,"Ssid empty, start Uc2 open softap");
      WiFi.softAP(mSSIDAP.c_str());
    }
    else if (password.isEmpty())
    {
      ESP_LOGI(TAG,"Ssid start %s open softap",ssid);
      WiFi.softAP(ssid.c_str());
    }
    else
    {
      ESP_LOGI(TAG,"Ssid start %s close softap",ssid);
      WiFi.softAP(ssid.c_str(), password.c_str());
    }
    ESP_LOGI(TAG,"Connected. IP: %s",WiFi.softAPIP().toString());
    //server = new WebServer(WiFi.softAPIP(),80);
}

void setup()
{
  if (mSSID != nullptr)
    ESP_LOGI(TAG,"mssid:%s pw:%s ap:%s", mSSID,mPWD,boolToChar(mAP));
  if (server !=  nullptr)
  {
     server->close();
     server = nullptr;
  }
  if(mSSID == "")
  {
    mAP = true;
    createAp(mSSIDAP,mPWD);
  }
  else if(mAP){
    createAp(mSSID,mPWD);
  }
  else
  {
    WiFi.softAPdisconnect();
    ESP_LOGI(TAG,"Connect to:%s",mSSID);
    WiFi.begin(mSSID.c_str(), mPWD.c_str());

    int nConnectTrials = 0;
    while (WiFi.status() != WL_CONNECTED && nConnectTrials <=10)
    {
      ESP_LOGI(TAG,"Wait for connection");
      delay(200);
      nConnectTrials += 1;
      // we can even make the ESP32 to sleep
    }
    if (nConnectTrials == 10)
    {
      ESP_LOGI(TAG,"failed to connect,Start softap");
      mAP = true;
      mSSID = mSSIDAP;
      mPWD = "";
      createAp(mSSIDAP,"");
    }
    else
    {
      ESP_LOGI(TAG,"Connected. IP: %s",WiFi.localIP());
      //server = new WebServer(WiFi.localIP(),80);
    }
  }
  server = new WebServer(80);
  if (jsonDocument == nullptr)
  {
    createJsonDoc();
  }
  setup_routing();
  server->begin();
  ESP_LOGI(TAG,"HTTP Running server  nullptr: %s jsondoc  nullptr: %s", boolToChar(server == nullptr),boolToChar(jsonDocument == nullptr));
}

void setup_routing()
{
  ESP_LOGI(TAG,"Setting up HTTP Routing");
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
  server->on(connectwifi_endpoint, HTTP_POST, RestApi::connectToWifi);
  server->on(reset_nv_flash_endpoint, HTTP_GET, RestApi::resetNvFLash);

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
  server->on(ledarr_get_endpoint, HTTP_GET, RestApi::Led_get);
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

  ESP_LOGI(TAG,"Setting up HTTP Routing END");
}
}
#endif
