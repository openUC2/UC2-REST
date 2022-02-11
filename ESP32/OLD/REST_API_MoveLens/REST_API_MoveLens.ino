#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>


const char* ssid = "Blynk";
const char* password = "12345678";  

String hostname = "ESPLENS";

/*Lens GPIO pins*/
int LENS_X_PIN = 25;
int LENS_Z_PIN = 26;

// default values for x/z lens' positions
int lens_x_int = 0;
int lens_z_int = 0;
int laser_int = 0;
int lens_x_offset = 0;
int lens_z_offset = 0;//1000;

// PWM Stuff
int pwm_resolution = 10;
int pwm_frequency = 312500/4;
int pwm_max = (int)pow(2, pwm_resolution);

// lens x-channel
int PWM_CHANNEL_X = 0;

// lens z-channel
int PWM_CHANNEL_Z = 1;

// Web server running on port 80
WebServer server(80);

// JSON data buffer
//StaticJsonDocument<2500> jsonDocument;
char buffer[2500];
DynamicJsonDocument jsonDocument(2048);


void setup() {
  Serial.begin(115200);
  
  // initiliaze connection
  connectToWiFi();
  setup_routing();

/* setup the PWM ports and reset them to 0*/
  ledcSetup(PWM_CHANNEL_X, pwm_frequency, pwm_resolution);
  ledcAttachPin(LENS_X_PIN, PWM_CHANNEL_X);
  ledcWrite(PWM_CHANNEL_X, 0);

  ledcSetup(PWM_CHANNEL_Z, pwm_frequency, pwm_resolution);
  ledcAttachPin(LENS_Z_PIN, PWM_CHANNEL_Z);
  ledcWrite(PWM_CHANNEL_Z, 0);


  // test lenses
  ledcWrite(PWM_CHANNEL_Z, 5000);
  ledcWrite(PWM_CHANNEL_X, 5000);
  delay(500);

  //Set the lenses to their offset level
  ledcWrite(PWM_CHANNEL_Z, lens_z_offset);
  ledcWrite(PWM_CHANNEL_X, lens_x_offset);

  Serial.println("Initialization finished.");


}

void loop() {
  server.handleClient();
 
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
      if(notConnectedCounter > 50) { // Reset board if not connected after 5s
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
  server.on("/lens_x", HTTP_POST, move_lens_x);
  server.on("/lens_z", HTTP_POST, move_lens_z);
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


/*


   Move Lenses


*/

void move_lens_x() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  Serial.println(body);
  deserializeJson(jsonDocument, body);


  // Get RGB components
  int value = jsonDocument["lens_value"];

  // apply a nonlinear look-up table
  int lensval = (value + lens_x_offset); // (int)(pow(((float)(value + lens_x_offset) / (float)pwm_max), 2) * (float)pwm_max);
  
  Serial.print("Lens (right) X is set to: ");
  Serial.print(lensval);
  Serial.println();

  // Respond to the client
  server.send(200, "application/json", "{}");

  // Perform action
  ledcWrite(PWM_CHANNEL_X, lensval);
}


void move_lens_z() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int value = jsonDocument["lens_value"];

  // apply a nonlinear look-up table
  int lensval = (value + lens_x_offset);
//  int lensval = (int)(pow(((float)(value + lens_x_offset) / (float)pwm_max), 2) * (float)pwm_max);
  
  Serial.print("Lens (right) Z is set to: ");
  Serial.print(lensval);
  Serial.println();

  // Respond to the client
  server.send(200, "application/json", "{}");

  // Perform action
  ledcWrite(PWM_CHANNEL_Z, lensval);
}


static inline int8_t sgn(int val) {
  if (val < 0) return -1;
  if (val == 0) return 0;
  return 1;
}
