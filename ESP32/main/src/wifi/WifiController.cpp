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

void WifiController::init_Spiffs()
{
  if (!SPIFFS.begin()) /* Démarrage du gestionnaire de fichiers SPIFFS */
  {
    Serial.println(F("Erreur SPIFFS..."));
    return;
  }

  /* Détection des fichiers présents sur l'Esp32 */
  File root = SPIFFS.open("/");    /* Ouverture de la racine */
  File file = root.openNextFile(); /* Ouverture du 1er fichier */
  while (file)                     /* Boucle de test de présence des fichiers - Si plus de fichiers la boucle s'arrête*/
  {
    Serial.print(F("File: "));
    Serial.println(file.name());
    file.close();
    file = root.openNextFile(); /* Lecture du fichier suivant */
  }
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
  /*return index page which is stored in serverIndex */

  Serial.println(F("Spinning up OTA server"));
  server->on("/", HTTP_GET, []()
             {
    wifi.server->sendHeader(F("Connection"), F("close"));
    wifi.server->send(200, "text/html", otaindex); });
  /*handling uploading firmware file */
  server->on(
      "/update", HTTP_POST, []()
      {
    wifi.server->sendHeader("Connection", "close");
    wifi.server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); },
      []()
      {
        HTTPUpload &upload = wifi.server->upload();
        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
          { // start with max available size
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          /* flashing firmware to ESP*/
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          }
          else
          {
            Update.printError(Serial);
          }
        }
      });
  server->begin();
  Serial.println(F("Starting OTA server on port: '82'"));
  Serial.println(("Visit http://%s:82", WiFi.localIP()));
}

void WifiController::setup_routing()
{
  // GET
  //  server.on("/temperature", getTemperature);
  // server.on("/env", getEnv);
  // https://www.survivingwithandroid.com/esp32-rest-api-esp32-api-server/
  Serial.println("Setting up HTTP Routing");
  wifi.server->on(state_act_endpoint, HTTP_POST, RestApi::State_act);
  wifi.server->on(state_get_endpoint, HTTP_POST, RestApi::State_get);
  wifi.server->on(state_set_endpoint, HTTP_POST, RestApi::State_set);

  wifi.server->on("/identity", RestApi::getIdentity);

  server->on("/ota", HTTP_GET, RestApi::ota);
  /*handling uploading firmware file */
  server->on("/update", HTTP_POST, RestApi::update, RestApi::upload);

  // Website
  SPIFFS.begin();
  wifi.server->on("/openapi.yaml", RestApi::handleSwaggerYaml);
  wifi.server->on("/index.html", RestApi::handleSwaggerUI);
  wifi.server->on("/swagger_standalone.js", RestApi::handlestandalone);
  wifi.server->on("/swagger-ui-bundle.js", RestApi::handleswaggerbundle);
  wifi.server->on("/swagger-ui.css", RestApi::handleswaggercss);
  SPIFFS.end();

  /*
  * server.on("/backbone-min.js", handlebackbone);
  server.on("/handlebars.min.js", handlehandlebars);
  server.on("/highlight.min.js", handlehighlight);
  server.on("/images/throbber.gif", handlethrobber);
  server.on("/jquery-1.8.0.min.js", handlejquery);
  server.on("/jquery.ba-bbq.min.js", handlejquerybbq);
  server.on("/json.min.js", handlejson);
  server.on("/jsoneditor.min.js", handlejsoneditor);
  server.on("/lodash.min.js", handlelodash);
  server.on("/logo_small.png", handlelogo_small);
  server.on("/marked.min.js", handlemarked);
  server.on("/reset.min.css", handlereset);
  server.on("/screen.css", handlescreen);
  server.on("/swagger-ui.min.js", handleswaggerui);
  */

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
  // start server
  wifi.server->begin();
}

#endif
