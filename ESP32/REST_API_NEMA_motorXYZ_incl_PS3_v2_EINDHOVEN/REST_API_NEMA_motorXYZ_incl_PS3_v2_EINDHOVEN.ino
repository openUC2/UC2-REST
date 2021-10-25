#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Stepper.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>

// Setup MOTOR
// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 120

// Brightfield
#define LED_ARRAY_PIN 2
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, LED_ARRAY_PIN,
                            NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                            NEO_GRB            + NEO_KHZ800);


// Filterslider
const int stepsPerRevolution = 2048;
int stp1 = 32;
int stp2 = 27;
int stp3 = 21;
int stp4 = 25;

Stepper stepper_focus = Stepper(stepsPerRevolution, stp1, stp2, stp3, stp4);

// setting up wifi parameters
const char *SSID = "Blynk"; //"BenMur"; //
const char *PWD = "12345678"; // "MurBen3128";//
String hostname = "ESPUC2";

/*LASER GPIO pin*/
int LED_PIN = 3;

int STEPPER_Z_FWD = 0;
int STEPPER_Z_BWD = 0;
int LED_ARRAY_val = 0;

int STATE = 0;
int STEPS = 200;
int SPEED = 0;

// Web server running on port 80
WebServer server(80);

// JSON data buffer
//StaticJsonDocument<2500> jsonDocument;
char buffer[5000];
DynamicJsonDocument jsonDocument(4096);

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


void setup_routing() {
  server.on("/matrix", HTTP_POST, handleMatrix);
  server.on("/move_z", HTTP_POST, move_z);
  
  // start server
  server.begin();

  // LED Matrix
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(255);
  matrix.fillScreen(255);
  matrix.show();
  delay(1000);
  matrix.fillScreen(0);
  matrix.show();
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


void move_z() {
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
  stepper_focus.setSpeed(speed);


  Serial.print("steps filter: ");
  Serial.print(steps);
  stepper_focus.step(steps );

  digitalWrite(stp1, LOW);
  digitalWrite(stp2, LOW);
  digitalWrite(stp3, LOW);
  digitalWrite(stp4, LOW);
}




void handleMatrix(){

  Serial.println("HandleMatrix");
  
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  Serial.println(body);

  int arraySize = jsonDocument["red"].size();   //get size of JSON Array
  Serial.print("\nSize of value array: ");
  Serial.println(arraySize);
  int Naxis = (int)sqrt(arraySize);
  for (int i = 0; i < arraySize; i++) { //Iterate through results
    int red = jsonDocument["red"][i];  //Implicit cast
    int green = jsonDocument["green"][i];  //Implicit cast
    int blue = jsonDocument["blue"][i];  //Implicit cast
    Serial.print(red);Serial.print(green);Serial.println(blue);
    int ix = i%Naxis;
    int iy = i/Naxis;
    set_led_RGB(ix, iy, red, green, blue);
  }

  // Respond to the client
  server.send(200, "application/json", "{}");
  
}

void set_led_RGB(int Nx, int Ny, int R, int G, int B)  {
  matrix.drawPixel(Nx, Ny, matrix.Color(R,   G,   B));
  matrix.show();
}


void setup() {
  Serial.begin(115200);

  /* set led and laser as output to control led on-off */
  pinMode(LED_PIN, OUTPUT);
  // Visualize, that ESP is on!
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Turing on");
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Turing off");
  delay(1000);

  /*
     Set target motor RPM.
  */
  Serial.print("start stepper z");
  stepper_focus.setSpeed(10);
  stepper_focus.step(1000);
  stepper_focus.step(-1000);

  // initiliaze connection
  connectToWiFi();
  setup_routing();
}

void loop() {
  server.handleClient();
}
