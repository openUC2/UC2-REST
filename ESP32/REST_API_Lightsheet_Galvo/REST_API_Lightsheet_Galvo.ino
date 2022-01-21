#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "pindef.h"

#define is_wifi true
#define is_ps3 true


// galvo
int x_start = 0;
int x_stop = 255;
int x_step = 5;
float x_delay = 100;

int y_start = 0;
int y_stop = 255;
int y_step = 1;
float y_delay = 0;
int t_start = 0;
boolean y_active = false;
boolean x_active = false;

int galvo_amplitude_y = 255;
int galvo_position_x = 0;


//#define home_wifi
#ifdef home_wifi
const char *ssid = "BenMur";
const char *password = "MurBen3128";
#else
const char* ssid = "Blynk";
const char* password = "12345678";
#endif


String hostname = "UC2_GALVO";

/*LASER GPIO pin*/
int LASER_PIN_PLUS = 14;// 22;
int LASER_PIN_MINUS = 0;//23;
int PWM_CHANNEL_LASER = 1;
int pwm_resolution = 15;
int pwm_frequency = 800000;//19000; //12000
int pwm_max = (int)pow(2, pwm_resolution);


// Web server running on port 80
WebServer server(80);

// JSON data buffer
//StaticJsonDocument<2500> jsonDocument;
char buffer[2500];
DynamicJsonDocument jsonDocument(2048);

int laserval = 1000;

void setup() {
  disableCore0WDT();
  disableCore1WDT();

  Serial.begin(115200);
  Serial.println("Start");

  // switch of the laser directly
  pinMode(LASER_PIN_PLUS, OUTPUT);
  pinMode(LASER_PIN_MINUS, OUTPUT);
  digitalWrite(LASER_PIN_PLUS, LOW);
  digitalWrite(LASER_PIN_MINUS, LOW);

  // initiliaze connection
  if (is_wifi) {
    connectToWiFi();
    setup_routing();
  }

  ledcSetup(PWM_CHANNEL_LASER, pwm_frequency, pwm_resolution);
  ledcAttachPin(LASER_PIN_PLUS, PWM_CHANNEL_LASER);
  ledcWrite(PWM_CHANNEL_LASER, 0);

  Serial.println("Initialization finished.");

  Serial.println("Start controlGalvoTask");
  xTaskCreatePinnedToCore(controlGalvoTask, "controlGalvoTask", 10000, NULL, 1, NULL, 1);
  Serial.println("Done with setting up Tasks");

}

void loop() {
  if (is_wifi) {
    server.handleClient();
  }


}



void controlGalvoTask( void * parameter ) {

  while (1) {
    if (true) {
      ledcWrite(PWM_CHANNEL_LASER, laserval );
      dacWrite(PIN_GALVO_Y,  galvo_amplitude_y * 1); 
      delay(1);
      digitalWrite(LASER_PIN_PLUS, LOW); //switch off laser again when roundtrip is reached
      dacWrite(PIN_GALVO_Y,  0); //abs(iy*2));//SineValues[iy]);
      delay(1);
    }
    else {
      for (int ix = 0; ix < 1; ix ++) {
        dacWrite(PIN_GALVO_X, ix * (127));
        delay(1);
        ledcWrite(PWM_CHANNEL_LASER, 1000);
        for (int iy = 0; iy < 2; iy ++) {
          dacWrite(PIN_GALVO_Y, iy * 255);
          delay(1);
        }
        ledcWrite(PWM_CHANNEL_LASER, 0);
      }
    }
  }
  vTaskDelete(NULL);
}




void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); //define hostname
  WiFi.begin(ssid, password);

  int notConnectedCounter = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    notConnectedCounter++;
    if (notConnectedCounter > 50) { // Reset board if not connected after 5s
      Serial.println("Resetting due to Wifi not connecting...");
      ESP.restart();
    }
  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}


/*
        RESTAPI RELATED
*/

void setup_routing() {
  // GET
  //  server.on("/temperature", getTemperature);
  //server.on("/env", getEnv);

  // POST
  server.on("/laser", HTTP_POST, set_laser);
  server.on("/galvo/position_x", HTTP_POST, set_galvo_positionx);
  server.on("/galvo/amplitude_y", HTTP_POST, set_galvo_amplitudey);
  server.on("/identify", identify);
  // start server
  server.begin();
}



void create_json(char *tag, float value, char *unit) {
  jsonDocument.clear();
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit;
}


void identify() {
  server.send(200, "application/json", "ESP32");
}


/*
    Set Laser
*/
void set_laser() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  laserval = jsonDocument["value"];

  //int laserval = (int)(pow(((float)(value) / (float)pwm_max), 2) * (float)pwm_max);
  // Respond to the client
  server.send(200, "application/json", "{}");

  // PErform action
  ledcWrite(PWM_CHANNEL_LASER, laserval );
  Serial.print("Laser is set to: ");
  Serial.print(laserval);
  Serial.println();
}

void run_laser(int laserval) {
  ledcWrite(PWM_CHANNEL_LASER, laserval );
}




static inline int8_t sgn(int val) {
  if (val < 0) return -1;
  if (val == 0) return 0;
  return 1;
}


void set_galvo_positionx(void) {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  galvo_position_x = jsonDocument["value"];
  dacWrite(PIN_GALVO_X, galvo_position_x);
  Serial.print("galvo_position_x: ");
  Serial.println(galvo_position_x);

  // Respond to the client
  server.send(200, "application/json", "{}");
}


void set_galvo_amplitudey(void) {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  galvo_amplitude_y = jsonDocument["value"];
  Serial.print("galvo_amplitude_y: ");
  Serial.println(galvo_amplitude_y);

  // Respond to the client
  server.send(200, "application/json", "{}");
}
