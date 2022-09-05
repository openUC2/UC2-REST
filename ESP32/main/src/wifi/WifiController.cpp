#include "WifiController.h"

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>


WifiController::WifiController()
{
  globalWifi = this;
}

WifiController::~WifiController()
{
  globalWifi = nullptr;
}

void WifiController::handelMessages()
{
  globalWifi->server->handleClient();
}

void WifiController::init_Spiffs()
{
  if (!SPIFFS.begin()) /* Démarrage du gestionnaire de fichiers SPIFFS */
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  /* Détection des fichiers présents sur l'Esp32 */
  File root = SPIFFS.open("/");    /* Ouverture de la racine */
  File file = root.openNextFile(); /* Ouverture du 1er fichier */
  while (file)                     /* Boucle de test de présence des fichiers - Si plus de fichiers la boucle s'arrête*/

  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile(); /* Lecture du fichier suivant */
  }
}

void WifiController::initWifiAP(const char *ssid) {
  Serial.print("Network SSID (AP): ");
  Serial.println(ssid);

  WiFi.softAP(ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}


void WifiController::joinWifi(const char *ssid, const char *password) {
  Serial.print("Connecting to ");
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

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}



void WifiController::autoconnectWifi(boolean isResetWifiSettings) {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  if (isResetWifiSettings) {
    Serial.println("First run => resetting Wifi Settings");
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
    Serial.println("Failed to connect");
    initWifiAP(mSSIDAP);
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...");
  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());

}



void WifiController::startserver() {
  /*return index page which is stored in serverIndex */

  Serial.println("Spinning up OTA server");
  server->on("/", HTTP_GET, []() {
    globalWifi->server->sendHeader("Connection", "close");
    globalWifi->server->send(200, "text/html", otaindex);
  });
  /*handling uploading firmware file */
  server->on("/update", HTTP_POST, []() {
    globalWifi->server->sendHeader("Connection", "close");
    globalWifi->server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = globalWifi->server->upload();
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
  Serial.println("Starting OTA server on port: '82'");
  Serial.println("Visit http://IPADDRESS_SCOPE:82");
}

//https://www.gabrielcsapo.com/arduino-web-server-esp-32/
bool loadFromSPIFFS(String path) {
  String dataType = "text/html";
 
  Serial.print("Requested page -> ");
  Serial.println(path);
  if (SPIFFS.exists(path)){
      File dataFile = SPIFFS.open(path, "r");
      if (!dataFile) {
          handleNotFound();
          return false;
      }
 
      if (globalWifi->server->streamFile(dataFile, dataType) != dataFile.size()) {
        Serial.println("Sent less data than expected!");
      }else{
          Serial.println("Page served!");
      }
 
      dataFile.close();
  }else{
      handleNotFound();
      return false;
  }
  return true;
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += globalWifi->server->uri();
  message += "\nMethod: ";
  message += (globalWifi->server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += globalWifi->server->args();
  message += "\n";
 
  for (uint8_t i = 0; i < globalWifi->server->args(); i++) {
    message += " " + globalWifi->server->argName(i) + ": " + globalWifi->server->arg(i) + "\n";
  }
 
  globalWifi->server->send(404, "text/plain", message);
}


void handleSwaggerYaml() { //Handler for the body path
    loadFromSPIFFS("/openapi.yaml");
}

void handleSwaggerUI() { //Handler for the body path 
    loadFromSPIFFS("/index.html");
}

    
void handlestandalone() { 
    loadFromSPIFFS("/swagger_standalone.js");
}

void handleswaggerbundle() { 
    loadFromSPIFFS("/swagger-ui-bundle.js");
}

void handleswaggercss() { 
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

  globalWifi->server->on(state_act_endpoint, HTTP_POST, State_act_fct_http_wrapper);
  globalWifi->server->on(state_get_endpoint, HTTP_POST, State_get_fct_http_wrapper);
  globalWifi->server->on(state_set_endpoint, HTTP_POST, State_set_fct_http_wrapper);

  // Website
  globalWifi->server->on("/openapi.yaml", handleSwaggerYaml);
  globalWifi->server->on("/index.html", handleSwaggerUI);
  globalWifi->server->on("/swagger_standalone.js", handlestandalone);
  globalWifi->server->on("/swagger-ui-bundle.js", handleswaggerbundle);
  globalWifi->server->on("/swagger-ui.css", handleswaggercss);


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
  globalWifi->server->on(motor_act_endpoint, HTTP_POST, FocusMotor_motor_act_fct_http_wrapper);
  globalWifi->server->on(motor_get_endpoint, HTTP_POST, FocusMotor_motor_get_fct_http_wrapper);
  globalWifi->server->on(motor_set_endpoint, HTTP_POST, FocusMotor_motor_set_fct_http_wrapper);


#ifdef IS_DAC
  globalWifi->server->on(dac_act_endpoint, HTTP_POST, Dac_act_fct_http_wrapper);
  globalWifi->server->on(dac_get_endpoint, HTTP_POST, Dac_get_fct_http_wrapper);
  globalWifi->server->on(dac_set_endpoint, HTTP_POST, Dac_set_fct_http_wrapper);
#endif

  globalWifi->server->on(laser_act_endpoint, HTTP_POST, Laser_act_fct_http_wrapper);
  globalWifi->server->on(laser_get_endpoint, HTTP_POST, Laser_get_fct_http_wrapper);
  globalWifi->server->on(laser_set_endpoint, HTTP_POST, Laser_set_fct_http_wrapper);

#ifdef IS_ANALOG
  globalWifi->server->on(analog_act_endpoint, HTTP_POST, Analog_act_fct_http_wrapper);
  globalWifi->server->on(analog_get_endpoint, HTTP_POST, Analog_get_fct_http_wrapper);
  globalWifi->server->on(analog_set_endpoint, HTTP_POST, Analog_set_fct_http_wrapper);
#endif

#ifdef IS_DIGITAL
  globalWifi->server->on(digital_act_endpoint, HTTP_POST, digital_act_fct_http);
  globalWifi->server->on(digital_get_endpoint, HTTP_POST, digital_get_fct_http);
  globalWifi->server->on(digital_set_endpoint, HTTP_POST, digital_set_fct_http);
#endif

globalWifi->server->on(ledarr_act_endpoint, HTTP_POST, Led_act_fct_http_wrapper);
globalWifi->server->on(ledarr_get_endpoint, HTTP_POST, Led_get_fct_http_wrapper);
globalWifi->server->on(ledarr_set_endpoint, HTTP_POST, Led_set_fct_http_wrapper);




  // start server
  globalWifi->server->begin();
}


