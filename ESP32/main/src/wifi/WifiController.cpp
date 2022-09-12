#include "../../config.h"
#ifdef IS_WIFI
#include "WifiController.h"

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

WifiController::WifiController(/* args */)
{
  wm = new WiFiManager();
  server = new WebServer(80);
}
WifiController::~WifiController(){
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

void WifiController::initWifiAP(const char *ssid) {
  Serial.print(F("Network SSID (AP): "));
  Serial.println(ssid);

  WiFi.softAP(ssid);
  Serial.print(F("AP IP address: "));
  Serial.println(WiFi.softAPIP());
}


void WifiController::joinWifi(const char *ssid, const char *password) {
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int nConnectTrials = 0;
  while (WiFi.status() != WL_CONNECTED) {
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



void WifiController::autoconnectWifi(boolean isResetWifiSettings) {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  if (isResetWifiSettings) {
    Serial.println(F("First run => resetting Wifi Settings"));
    wm->resetSettings();
  }
  wm->setHostname(hostname);
  wm->setConfigPortalBlocking(false);
  //wm.setConfigPortalBlocking(false);
  //wm.setConfigPortalTimeout(90); // auto close configportal after n seconds
  wm->setConnectTimeout(10);

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result
  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm->autoConnect(mSSIDAP); // password protected ap


  if (!res) {
    Serial.println(F("Failed to connect"));
    initWifiAP(mSSIDAP);
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println(F("connected..."));
  }

  Serial.print(F("Connected. IP: "));
  Serial.println(WiFi.localIP());
}



void WifiController::startserver() {
  /*return index page which is stored in serverIndex */

  Serial.println(F("Spinning up OTA server"));
  server->on("/", HTTP_GET, []() {
    wifi.server->sendHeader(F("Connection"), F("close"));
    wifi.server->send(200, "text/html", otaindex);
  });
  /*handling uploading firmware file */
  server->on("/update", HTTP_POST, []() {
    wifi.server->sendHeader("Connection", "close");
    wifi.server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = wifi.server->upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server->begin();
  Serial.println(F("Starting OTA server on port: '82'"));
  Serial.println(("Visit http://%s:82", WiFi.localIP()));
}



void WifiController::handleNotFound() {
    String message = F("File Not Found\n\n");
    message += F("URI: ");
    message += wifi.server->uri();
    message += F("\nMethod: ");
    message += (wifi.server->method() == HTTP_GET) ? "GET" : "POST";
    message += F("\nArguments: ");
    message += wifi.server->args();
    message += F("\n");
  
    for (uint8_t i = 0; i < wifi.server->args(); i++) {
        message += " " + wifi.server->argName(i) + ": " + wifi.server->arg(i) + "\n";
    }
  
    wifi.server->send(404, "text/plain", message);
}

//https://www.gabrielcsapo.com/arduino-web-server-esp-32/
bool WifiController::loadFromSPIFFS(String path) {
  String dataType = "text/html";
 
  Serial.print(F("Requested page -> "));
  Serial.println(path);
  if (SPIFFS.exists(path)){
      File dataFile = SPIFFS.open(path, "r");
      if (!dataFile) {
          handleNotFound();
          return false;
      }
 
      if (wifi.server->streamFile(dataFile, dataType) != dataFile.size()) {
        Serial.println(F("Sent less data than expected!"));
      }else{
          Serial.println(F("Page served!"));
      }
 
      dataFile.close();
  }else{
      handleNotFound();
      return false;
  }
  return true;
}

void WifiController::handleSwaggerYaml() { //Handler for the body path
      loadFromSPIFFS("/openapi.yaml");
  }

  void WifiController::handleSwaggerUI() { //Handler for the body path 
      loadFromSPIFFS("/index.html");
  }

      
  void WifiController::handlestandalone() { 
      loadFromSPIFFS("/swagger_standalone.js");
  }

  void WifiController::handleswaggerbundle() { 
      loadFromSPIFFS("/swagger-ui-bundle.js");
  }

  void WifiController::handleswaggercss() { 
      loadFromSPIFFS("/swagger-ui.css");
  }

  /*
    Define Endpoints for HTTP REST API
  */


  void WifiController::setup_routing() {
    // GET
    //  server.on("/temperature", getTemperature);
    //server.on("/env", getEnv);
    // https://www.survivingwithandroid.com/esp32-rest-api-esp32-api-server/
    Serial.println("Setting up HTTP Routing");
    wifi.server->on(state_act_endpoint, HTTP_POST, State_act);
    wifi.server->on(state_get_endpoint, HTTP_POST, State_get);
    wifi.server->on(state_set_endpoint, HTTP_POST, State_set);

    wifi.server->on("/identity", getIdentity);

    server->on("/ota", HTTP_GET, []() {
      wifi.server->sendHeader("Connection", "close");
      wifi.server->send(200, "text/html", otaindex);
    });
  /*handling uploading firmware file */
    server->on("/update", HTTP_POST, []() {
      wifi.server->sendHeader("Connection", "close");
      wifi.server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = wifi.server->upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    });

    // Website
    wifi.server->on("/openapi.yaml", handleSwaggerYaml);
    wifi.server->on("/index.html", handleSwaggerUI);
    wifi.server->on("/swagger_standalone.js", handlestandalone);
    wifi.server->on("/swagger-ui-bundle.js", handleswaggerbundle);
    wifi.server->on("/swagger-ui.css", handleswaggercss);


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
    wifi.server->on(motor_act_endpoint, HTTP_POST, FocusMotor_act);
    wifi.server->on(motor_get_endpoint, HTTP_POST, FocusMotor_get);
    wifi.server->on(motor_set_endpoint, HTTP_POST, FocusMotor_set);
#endif

#ifdef IS_DAC
    wifi.server->on(dac_act_endpoint, HTTP_POST, Dac_act);
    wifi.server->on(dac_get_endpoint, HTTP_POST, Dac_get);
    wifi.server->on(dac_set_endpoint, HTTP_POST, Dac_set);
#endif

#ifdef IS_LASER
    wifi.server->on(laser_act_endpoint, HTTP_POST, Laser_act);
    wifi.server->on(laser_get_endpoint, HTTP_POST, Laser_get);
    wifi.server->on(laser_set_endpoint, HTTP_POST, Laser_set);
#endif

#ifdef IS_ANALOG
    wifi.server->on(analog_act_endpoint, HTTP_POST, Analog_act);
    wifi.server->on(analog_get_endpoint, HTTP_POST, Analog_get);
    wifi.server->on(analog_set_endpoint, HTTP_POST, Analog_set);
#endif

#ifdef IS_DIGITAL
    wifi.server->on(digital_act_endpoint, HTTP_POST, Digital_act);
    wifi.server->on(digital_get_endpoint, HTTP_POST, Digital_get);
    wifi.server->on(digital_set_endpoint, HTTP_POST, Digital_set);
#endif
  
#ifdef IS_PID
    wifi.server->on(PID_act_endpoint, HTTP_POST, Pid_act);
    wifi.server->on(PID_get_endpoint, HTTP_POST, Pid_get);
    wifi.server->on(PID_set_endpoint, HTTP_POST, Pid_set);
#endif

#ifdef IS_LED
  wifi.server->on(ledarr_act_endpoint, HTTP_POST, Led_act);
  wifi.server->on(ledarr_get_endpoint, HTTP_POST, Led_get);
  wifi.server->on(ledarr_set_endpoint, HTTP_POST, Led_set);
#endif

#ifdef IS_SLM
  wifi.server->on(slm_act_endpoint, HTTP_POST, Slm_act);
  wifi.server->on(slm_get_endpoint, HTTP_POST, Slm_get);
  wifi.server->on(slm_set_endpoint, HTTP_POST, Slm_set);
#endif

  wifi.server->on(config_act_endpoint, HTTP_POST, Config_act);
  wifi.server->on(config_get_endpoint, HTTP_POST, Config_get);
  wifi.server->on(config_set_endpoint, HTTP_POST, Config_set);
  // start server
  wifi.server->begin();
}


void WifiController::getIdentity() {
  //if(DEBUG) Serial.println("Get Identity");
    wifi.server->send(200, "application/json", state.identifier_name);
}

    void WifiController::deserialize()
    {
      String body = (*wifi.server).arg("plain");
      deserializeJson((*wifi.jsonDocument), body);
    }

    void WifiController::serialize()
    {
      serializeJson((*wifi.jsonDocument), wifi.output);
      (*wifi.server).send(200, "application/json", wifi.output);
    }

#ifdef IS_MOTOR
    void WifiController::FocusMotor_act()
    {
      deserialize();
      motor.act();
      serialize();
    }

    void WifiController::FocusMotor_get()
    {
      deserialize();
      motor.get();
      serialize();
    }

    void WifiController::FocusMotor_set()
    {
      deserialize();
      motor.set();
      serialize();
    }
#endif

#ifdef IS_LASER
    void WifiController::Laser_act()
    {
      deserialize();
      laser.act();
      serialize();
    }

    void WifiController::Laser_get()
    {
      deserialize();
      laser.get();
      serialize();
    }

    void WifiController::Laser_set()
    {
      deserialize();
      laser.set();
      serialize();
    }
#endif

#ifdef IS_DAC
    void WifiController::Dac_act()
    {
      deserialize();
      dac.act();
      serialize();
    }

    void WifiController::Dac_get()
    {
      deserialize();
      dac.get();
      serialize();
    }

    void WifiController::Dac_set()
    {
      deserialize();
      dac.set();
      serialize();
    }
#endif
#ifdef IS_LED
    void WifiController::Led_act()
    {
      deserialize();
      led.act();
      serialize();
    }

    void WifiController::Led_get()
    {
      deserialize();
      led.get();
      serialize();
    }

    void WifiController::Led_set()
    {
      deserialize();
      led.set();
      serialize();
    }
#endif

    void WifiController::State_act()
    {
      deserialize();
      state.act();
      serialize();
    }

    void WifiController::State_get()
    {
      deserialize();
      state.get();
      serialize();
    }

    void WifiController::State_set()
    {
      deserialize();
      state.set();
      serialize();
    }
#ifdef IS_ANALOG
    void WifiController::Analog_act()
    {
      deserialize();
      analog.act();
      serialize();
    }

    void WifiController::Analog_get()
    {
      deserialize();
      analog.get();
      serialize();
    }

    void WifiController::Analog_set()
    {
      deserialize();
      analog.set();
      serialize();
    }
#endif
#ifdef IS_DIGITAL

    void WifiController::Digital_act()
    {
      deserialize();
      DynamicJsonDocument *doc = wifi.jsonDocument;
      digital.act(doc);
      serialize();
    }

    void WifiController::Digital_get()
    {
      deserialize();
      digital.get(wifi.jsonDocument);
      serialize();
    }

    void WifiController::Digital_set()
    {
      deserialize();
      digital.set(wifi.jsonDocument);
      serialize();
    }
#endif
#ifdef IS_PID
    void WifiController::Pid_act()
    {
      deserialize();
      pid.act();
      serialize();
    }

    void WifiController::Pid_get()
    {
      deserialize();
      pid.get();
      serialize();
    }

    void WifiController::Pid_set()
    {
      deserialize();
      pid.set();
      serialize();
    }
#endif  
    void WifiController::Config_act()
    {
      deserialize();
      config.act();
      serialize();
    }

    void WifiController::Config_get()
    {
      deserialize();
      config.get();
      serialize();
    }

    void WifiController::Config_set()
    {
      deserialize();
      config.set();
      serialize();
    }
#ifdef IS_SLM
    void WifiController::Slm_act()
    {
      deserialize();
      slm.act();
      serialize();
    }

    void WifiController::Slm_get()
    {
      deserialize();
      slm.get();
      serialize();
    }

    void WifiController::Slm_set()
    {
      deserialize();
      slm.set();
      serialize();
    }
#endif
#endif

