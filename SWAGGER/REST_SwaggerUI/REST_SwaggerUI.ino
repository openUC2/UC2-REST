#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "A4988.h"
#include "Stepper.h"

WebServer server(80);

// JSON data buffer
//StaticJsonDocument<2500> jsonDocument;
char buffer[2500];
DynamicJsonDocument jsonDocument(2048);


const char* ssid = "Blynk";
const char* password =  "12345678";


// motor stuff
int glob_motor_steps[] = {0, 0, 0};
#define ENABLE 13
//A4988 stepper_x(MOTOR_STEPS, PIN_DIR_X, PIN_STEP_X, SLEEP, MS1, MS2, MS3);
#define MOTOR_STEPS 200
#define PIN_DIR_X 2
#define PIN_STEP_X 3
int speed = 1;
int steps = 1;
A4988 stepper_x(MOTOR_STEPS, PIN_DIR_X, PIN_STEP_X, 0, 0, 0, 0);
  
void setup() {
  Serial.begin(115200);
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }


  WiFi.begin(ssid, password);  //Connect to the WiFi network

  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection

    delay(500);
    Serial.println("Waiting to connect...");

  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP
  server.enableCORS();

  // serve REST
  server.on("/stepper/move", HTTP_GET, handleStepper);
  server.on("/openapi.json", handleSwaggerJson);
  server.on("/index.html", handleSwaggerUI);

  server.begin(); //Start the server
  Serial.println("Server listening");

}

// Start main operation
void loop() {
  server.handleClient(); //Handling of incoming requests
}


//Handler for the body path
void handleStepper() { 
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Some documentation
  String tag = "Stepper Motor";
  String summary = "Control a bipolar stepper motor";
  String description = "";

  // Extract Values and move 
  int steps = jsonDocument["steps"];
  int speed = jsonDocument["speed"];

  // action
  run_motor_x(steps, speed);

  char * answer = "[{\"value\":\"10.5\",\"timestamp\":\"22/10/2017 10:10\"},{\"value\":\"11.0\",\"timestamp\":\"22/10/2017 10:20\"}]";
  server.send(200, "application/json", answer);

}

//Handler for the body path
void handleSwaggerJson() { 
  File file = SPIFFS.open("/openapi.json", "r");  
  size_t sent = server.streamFile(file, "application/json");  
  file.close();  
  return;  
}

//Handler for the body path
void handleSwaggerUI() { 
  File file = SPIFFS.open("/index.html", "r");  
  size_t sent = server.streamFile(file, "text/html");  
  file.close();  
}


// misc


void run_motor_x(int steps, int speed) {
  if (steps == 0) {
    // run unsupervised
    glob_motor_steps[0] = sgn(speed) * 10;
  }
  else {
    // run only the number of steps we want
    glob_motor_steps[0] = 0;
    digitalWrite(ENABLE, LOW);
    stepper_x.begin(abs(speed));
    stepper_x.rotate(steps);
    digitalWrite(ENABLE, HIGH);
  }
}

static inline int8_t sgn(int val) {
  if (val < 0) return -1;
  if (val == 0) return 0;
  return 1;
}
