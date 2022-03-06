#ifdef IS_WIFI

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>


void init_Spiffs()
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



void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(mSSID);

  WiFi.begin(mSSID, mPWD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
    // we can even make the ESP32 to sleep
  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}
/*
void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(mSSID);
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); //define hostname
  WiFi.begin(mSSID, mPWD);

  int notConnectedCounter = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("Wifi connecting...");
    notConnectedCounter++;
    if (notConnectedCounter > 50) { // Reset board if not connected after 5s
      Serial.println("Resetting due to Wifi not connecting...");
      ESP.restart();
    }
  }
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}
*/

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
 
      if (server.streamFile(dataFile, dataType) != dataFile.size()) {
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
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
 
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
 
  server.send(404, "text/plain", message);
}


void handleSwaggerJson() { //Handler for the body path
    loadFromSPIFFS("/openapi.json");
}

void handleSwaggerUI() { //Handler for the body path 
    loadFromSPIFFS("/index.html");
}


void handlebackbone() { //Handler for the body path
    loadFromSPIFFS("/backbone-min.js");
}

void handlehandlebars() { 
    loadFromSPIFFS("/handlebars.min.js");
}

void handlehighlight() { 
    loadFromSPIFFS("/highlight.min.js");
}

void handlethrobber() { 
    loadFromSPIFFS("/images/throbber.gif");
}

void handlejquery() { 
    loadFromSPIFFS("/jquery-1.8.0.min.js");
}

void handlejquerybbq() { 
    loadFromSPIFFS("/jquery.ba-bbq.min.js");
}

void handlejson() { 
    loadFromSPIFFS("/json.min.js");
}

void handlejsoneditor() { 
    loadFromSPIFFS("/jsoneditor.min.js");
}

void handlelodash() { 
    loadFromSPIFFS("/lodash.min.js");
}

void handlelogo_small() { 
    loadFromSPIFFS("/logo_small.png");
}

void handlemarked() { 
    loadFromSPIFFS("/marked.min.js");
}

void handlereset() { 
    loadFromSPIFFS("/reset.min.css");
}

void handlescreen() { 
    loadFromSPIFFS("/screen.css");
}

void handleswaggerui() { 
    loadFromSPIFFS("/swagger-ui.min.js");
}



/*
   Define Endpoints for HTTP REST API
*/


void setup_routing() {
  // GET
  //  server.on("/temperature", getTemperature);
  //server.on("/env", getEnv);
  // https://www.survivingwithandroid.com/esp32-rest-api-esp32-api-server/

  server.on(state_act_endpoint, HTTP_POST, state_act_fct_http);
  server.on(state_get_endpoint, HTTP_POST, state_get_fct_http);
  server.on(state_set_endpoint, HTTP_POST, state_set_fct_http);

  // Website
  server.on("/openapi.json", handleSwaggerJson);
  server.on("/index.html", handleSwaggerUI);
  server.on("/backbone-min.js", handlebackbone);
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


#ifdef IS_MOTOR
  // POST
  server.on(motor_act_endpoint, HTTP_POST, x);
  server.on(motor_get_endpoint, HTTP_POST, motor_get_fct_http);
  server.on(motor_set_endpoint, HTTP_POST, motor_set_fct_http);
#endif

#ifdef IS_DAC
  server.on(dac_act_endpoint, HTTP_POST, dac_act_fct_http);
  server.on(dac_get_endpoint, HTTP_POST, dac_get_fct_http);
  server.on(dac_set_endpoint, HTTP_POST, dac_set_fct_http);
#endif

#ifdef IS_LASER
  server.on(laser_act_endpoint, HTTP_POST, LASER_act_fct_http);
  server.on(laser_get_endpoint, HTTP_POST, LASER_get_fct_http);
  server.on(laser_set_endpoint, HTTP_POST, LASER_set_fct_http);
#endif

#ifdef IS_ANALOGOUT
  server.on(analogout_act_endpoint, HTTP_POST, analogout_act_fct_http);
  server.on(analogout_get_endpoint, HTTP_POST, analogout_get_fct_http);
  server.on(analogout_set_endpoint, HTTP_POST, analogout_set_fct_http);
#endif

#ifdef IS_LEDARR
  server.on(ledarr_act_endpoint, HTTP_POST, ledarr_act_fct_http);
  server.on(ledarr_get_endpoint, HTTP_POST, ledarr_get_fct_http);
  server.on(ledarr_set_endpoint, HTTP_POST, ledarr_set_fct_http);
#endif




  // start server
  server.begin();
}
#endif
