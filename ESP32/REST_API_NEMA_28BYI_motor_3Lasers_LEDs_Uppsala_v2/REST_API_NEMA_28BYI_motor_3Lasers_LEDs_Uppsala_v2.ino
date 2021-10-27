#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Stepper.h>
#include <Adafruit_NeoPixel.h>

// Setup MOTOR
// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 120

// Brightfield
#define NEOLED_PIN    5
#define LED_COUNT 1
Adafruit_NeoPixel strip(LED_COUNT, NEOLED_PIN, NEO_GRB + NEO_KHZ800);

#define SWITCH_FILTER 22

#define DIR 2
#define STEP 4
#define SLEEP 13 // optional (just delete SLEEP from everywhere if not used)
#include "A4988.h"
#define MS1 10
#define MS2 11
#define MS3 12
A4988 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MS1, MS2, MS3);

// Filterslider
const int stepsPerRevolution = 2048;
int stp1 = 27;
int stp2 = 25;
int stp3 = 26;
int stp4 = 33;

Stepper stepper_filter = Stepper(stepsPerRevolution, stp1, stp2, stp3, stp4);


// setting up wifi parameters
const char *SSID = "Blynk"; //"BenMur"; //
const char *PWD = "12345678"; // "MurBen3128";//
String hostname = "ESPLENS";

/*LASER GPIO pin*/
int LASER_PIN_red = 18;// 22;
int LASER_PIN_green = 19;
int LASER_PIN_blue = 21;
int LED_PIN = 4;


///* topics - DON'T FORGET TO REGISTER THEM! */
int LED_val = 0;
int LASER_val_green = 0;
int LASER_val_blue = 0;
int LENS_VIBRATE = 0;
int LASER_val_red = 0;

int STEPPER_Z_FWD = 0;
int STEPPER_Z_BWD = 0;
int LED_ARRAY_val = 0;

int STATE = 0;
int STEPS = 200;
int SPEED = 0;


// PWM Stuff
int pwm_resolution = 15;
int pwm_frequency = 800000;//19000; //12000
int pwm_max = (int)pow(2, pwm_resolution);
// lens x-channel
int PWM_CHANNEL_LASER_GREEN = 0;

// lens z-channel
int PWM_CHANNEL_LASER_BLUE = 1;

// laser-channel
int PWM_CHANNEL_LASER_RED = 2;

// LED-cahnnel 
int PWM_CHANNEL_LED_WHITE = 3;

// env variable
float temperature;

// Web server running on port 80
WebServer server(80);

// JSON data buffer
//StaticJsonDocument<2500> jsonDocument;
char buffer[2500];
DynamicJsonDocument jsonDocument(2048);


void setup() {
  Serial.begin(115200);

  /* set led and laser as output to control led on-off */
  pinMode(LED_PIN, OUTPUT);

  // switch of the laser directly
  pinMode(LASER_PIN_red, OUTPUT);
  pinMode(LASER_PIN_green, OUTPUT);
  pinMode(LASER_PIN_blue, OUTPUT);
  digitalWrite(LASER_PIN_red, LOW);
  digitalWrite(LASER_PIN_green, LOW);
  digitalWrite(LASER_PIN_blue, LOW);

  // Brightfield LEd
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)
  set_led_RGB(255, 255, 255);
  // Visualize, that ESP is on!
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Turing on");

  delay(1000);
  set_led_RGB(0, 0, 0);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Turing off");
  delay(1000);

  /*
     Set target motor RPM.
  */
  stepper.begin(RPM);
  pinMode(SWITCH_FILTER, INPUT_PULLUP);

  // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
  // stepper.setEnableActiveState(LOW);
  stepper.enable();
  stepper.setMicrostep(1);  // Set microstep mode to 1:1
  Serial.print("start stepper z");
  stepper.move(100);
  stepper.move(-100);


  // initiliaze connection
  connectToWiFi();
  setup_routing();



  /* setup the PWM ports and reset them to 0*/
  ledcSetup(PWM_CHANNEL_LASER_GREEN, pwm_frequency, pwm_resolution);
  ledcAttachPin(LASER_PIN_green, PWM_CHANNEL_LASER_GREEN);
  ledcWrite(PWM_CHANNEL_LASER_GREEN, 0);

  ledcSetup(PWM_CHANNEL_LASER_BLUE, pwm_frequency, pwm_resolution);
  ledcAttachPin(LASER_PIN_blue, PWM_CHANNEL_LASER_BLUE);
  ledcWrite(PWM_CHANNEL_LASER_BLUE, 40000);

  ledcSetup(PWM_CHANNEL_LASER_RED, pwm_frequency, pwm_resolution);
  ledcAttachPin(LASER_PIN_red, PWM_CHANNEL_LASER_RED);
  ledcWrite(LASER_PIN_red, 0);

  ledcSetup(PWM_CHANNEL_LED_WHITE, pwm_frequency, pwm_resolution);
  ledcAttachPin(LED_PIN, PWM_CHANNEL_LED_WHITE);
  ledcWrite(LED_PIN, 0);

  Serial.println("Initialization finished.");

  // One complete revolution is 360°
  stepper.rotate(360);     // forward revolution
  stepper.rotate(-360);    // reverse revolution

}

void loop() {
  server.handleClient();
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); //define hostname
  WiFi.begin(SSID, PWD);

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


/*

        RESTAPI RELATED

*/

void setup_routing() {

  // POST
  server.on("/led", HTTP_POST, set_led);
  server.on("/led_white", HTTP_POST, set_led_white);
  server.on("/move_z", HTTP_POST, move_z);
  server.on("/move_filter", HTTP_POST, move_filter);
  server.on("/laser_green", HTTP_POST, set_laser_green);
  server.on("/laser_blue", HTTP_POST, set_laser_blue);
  server.on("/laser_red", HTTP_POST, set_laser_red);
  server.on("/switch_filter", HTTP_POST, switch_filters);
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

void identify(){
  server.send(200, "application/json", "ESP32");
}
void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit;
}



/*


    Set Laser

*/

void set_laser_red() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int value = jsonDocument["value"];

  // apply a nonlinear look-up table
  int laserval =  value; 
  //int laserval = (int)(pow(((float)(value) / (float)pwm_max), 2) * (float)pwm_max);
  // Respond to the client
  server.send(200, "application/json", "{}");

  // PErform action
  ledcWrite(PWM_CHANNEL_LASER_RED, laserval );
  Serial.print("Laser (red) is set to: ");
  Serial.print(laserval);
  Serial.println();
}



void set_laser_green() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int value = jsonDocument["value"];

  // apply a nonlinear look-up table
  int laserval = value;
  //int laserval = (int)(pow(((float)(value) / (float)pwm_max), 2) * (float)pwm_max);
  // Respond to the client
  server.send(200, "application/json", "{}");

  // PErform action
  ledcWrite(PWM_CHANNEL_LASER_GREEN, laserval );
  Serial.print("Laser (green) is set to: ");
  Serial.print(laserval);
  Serial.println();
}


void set_laser_blue() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int value = jsonDocument["value"];

  // apply a nonlinear look-up table
  int laserval = value;
  //int laserval = (int)(pow(((float)(value) / (float)pwm_max), 2) * (float)pwm_max);
  // Respond to the client
  server.send(200, "application/json", "{}");

  // PErform action
  ledcWrite(PWM_CHANNEL_LASER_BLUE, laserval );
  Serial.print("Laser (blue) is set to: ");
  Serial.print(laserval);
  Serial.println();
}


void set_led_white() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int value = jsonDocument["value"];

  // apply a nonlinear look-up table
  int ledval = value;
  //int laserval = (int)(pow(((float)(value) / (float)pwm_max), 2) * (float)pwm_max);
  // Respond to the client
  server.send(200, "application/json", "{}");

  // PErform action
  ledcWrite(PWM_CHANNEL_LED_WHITE, ledval);
  Serial.print("LED (white) is set to: ");
  Serial.print(ledval);
  Serial.println();
}


/*

   Move Motors

*/

void move_z() {

  Serial.println("move_Z");
  server.send(200, "application/json", "{}");
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int steps = jsonDocument["steps"];
  int speed = jsonDocument["speed"];

  // Set the speed to 5 rpm:
  stepper.begin(speed);
  Serial.print("steps x: ");
  Serial.print(steps);
  stepper.move(steps);

  // Respond to the client


}


/*
   Set LED
*/
void set_led() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int red = jsonDocument["red"];
  int green = jsonDocument["green"];
  int blue = jsonDocument["blue"];

  set_led_RGB(red, green, blue);

  // Respond to the client
  server.send(200, "application/json", "{}");
}

/*
   Move Filter
*/


void move_filter() {

  Serial.println("move_filter");
  server.send(200, "application/json", "{}");
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  server.send(200, "application/json", "{}");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int steps = jsonDocument["steps"];
  int speed = jsonDocument["speed"];

  // Set the speed to 5 rpm:
  stepper_filter.setSpeed(speed);


  Serial.print("steps filter: ");
  Serial.print(steps);
  stepper_filter.step(steps );

  digitalWrite(stp1, LOW);
  digitalWrite(stp2, LOW);
  digitalWrite(stp3, LOW);
  digitalWrite(stp4, LOW);
}


void set_led_RGB(int R, int G, int B)  {
  strip.setPixelColor(0, strip.Color(R,   G,   B));
  strip.show();
}



void switch_filters() {
  Serial.println("Start switching Filters ...");

  int val_switch_filter = 0;
  int time_start = millis();
   if (server.hasArg("plain") == false) {
    Serial.println("Something went wrong...");
  }  
  stepper_filter.setSpeed(15);
    
  while (not val_switch_filter) {
    Serial.println(val_switch_filter);
    if ((millis() - time_start) > 3000)
      val_switch_filter = digitalRead(SWITCH_FILTER);
    stepper_filter.step(1);
  }
  digitalWrite(stp1, LOW);
  digitalWrite(stp2, LOW);
  digitalWrite(stp3, LOW);
  digitalWrite(stp4, LOW);


  Serial.println("Filter Switched.");
  // Respond to the client
  server.send(200, "application/json", "{}");

}
