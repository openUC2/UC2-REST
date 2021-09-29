#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

/*
 * 
import urllib
import cv2
import numpy as np

url='http://192.168.8.100/cam-hi.jpg'

while True:
    imgResp=urllib.request.urlopen(url)
    imgNp=np.array(bytearray(imgResp.read()),dtype=np.uint8)
    img=cv2.imdecode(imgNp,-1)

    # all the opencv processing is done here
    cv2.imshow('test',img)
    if ord('q')==cv2.waitKey(10):
        exit(0)
        
 */
#include <Stepper.h>
// Define number of steps per rotation:
const int stepsPerRevolution = 2 * 2048;
const int Nsteps = 10000;
Stepper stepper_x = Stepper(stepsPerRevolution, 4, 25, 32, 27);
Stepper stepper_y = Stepper(stepsPerRevolution, 26, 18, 19, 23);

// setting up wifi parameters
const char *SSID = "Blynk"; //"BenMur"; //
const char *PWD = "12345678"; // "MurBen3128";//
String hostname = "ESPLENS";

/*LASER GPIO pin*/
int LASER_PIN_PLUS = 27;// 22;
int LASER_PIN_MINUS = 0;//23;
int LED_PIN = 2;

/*Lens GPIO pins*/
int LENS_X_PIN = 21;
int LENS_Z_PIN = 22;

///* topics - DON'T FORGET TO REGISTER THEM! */
int LED_val = 0;
int LENS_X_val = 0;
int LENS_Z_val = 0;
int LENS_VIBRATE = 0;
int LASER_val = 0;
int LASER_val_2 = 0;
int STEPPER_Z_FWD = 0;
int STEPPER_Z_BWD = 0;
int LED_ARRAY_val = 0;
int LENS_X_SOFI = 0;
int LENS_Z_SOFI = 0;
int STATE = 0;
int STEPS = 200;
int SPEED = 0;

int lensval_x = 0;
int lensval_z = 0;
int laser_diff = 0;
// global switch for vibrating the lenses
int sofi_periode = 100;  // ms
int sofi_amplitude_x = 0;   // how many steps +/- ?
int sofi_amplitude_z = 0;   // how many steps +/- ?

// default values for x/z lens' positions
int lens_x_int = 0;
int lens_z_int = 0;
int laser_int = 0;
int lens_x_offset = 0;
int lens_z_offset = 0;//1000;

boolean is_sofi_x = false;
boolean is_sofi_z = false;

// PWM Stuff
int pwm_resolution = 15;
int pwm_frequency = 800000;//19000; //12000
int pwm_max = (int)pow(2, pwm_resolution);
// lens x-channel
int PWM_CHANNEL_X = 0;

// lens z-channel
int PWM_CHANNEL_Z = 1;

// laser-channel
int PWM_CHANNEL_LASER = 2;

// laser-channel
int PWM_CHANNEL_LASER_2 = 3;


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

  // initiliaze connection
  connectToWiFi();
  setup_routing();

  /* set led and laser as output to control led on-off */
  pinMode(LED_PIN, OUTPUT);

  // switch of the laser directly
  pinMode(LASER_PIN_MINUS, OUTPUT);
  pinMode(LASER_PIN_MINUS, OUTPUT);
  digitalWrite(LASER_PIN_PLUS, LOW);
  digitalWrite(LASER_PIN_MINUS, LOW);

  // Visualize, that ESP is on!
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);

  /* setup the PWM ports and reset them to 0*/
  ledcSetup(PWM_CHANNEL_X, pwm_frequency, pwm_resolution);
  ledcAttachPin(LENS_X_PIN, PWM_CHANNEL_X);
  ledcWrite(PWM_CHANNEL_X, 0);

  ledcSetup(PWM_CHANNEL_Z, pwm_frequency, pwm_resolution);
  ledcAttachPin(LENS_Z_PIN, PWM_CHANNEL_Z);
  ledcWrite(PWM_CHANNEL_Z, 0);

  ledcSetup(PWM_CHANNEL_LASER, pwm_frequency, pwm_resolution);
  ledcAttachPin(LASER_PIN_PLUS, PWM_CHANNEL_LASER);
  ledcWrite(PWM_CHANNEL_LASER, 0);

  ledcWrite(PWM_CHANNEL_LASER_2, 0);

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
  Serial.println(SSID);  
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); //define hostname
  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
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
  server.on("/led", HTTP_POST, mset_led);
  server.on("/move_x", HTTP_POST, move_x);
  server.on("/move_y", HTTP_POST, move_y);
  server.on("/lens_x", HTTP_POST, move_lens_x);
  server.on("/lens_z", HTTP_POST, move_lens_z);
  server.on("/laser", HTTP_POST, set_laser);

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
void getTemperature() {
  Serial.println("Get temperature");
  create_json("temperature", temperature, "°C");
  server.send(200, "application/json", buffer);
}
void getEnv() {
  Serial.println("Get env");
  jsonDocument.clear();
  add_json_object("temperature", temperature, "°C");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}
*/



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
  int lensval = (int)(pow(((float)(value + lens_x_offset) / (float)pwm_max), 2) * (float)pwm_max);
  ledcWrite(PWM_CHANNEL_X, lensval);
  Serial.print("Lens (right) X is set to: ");
  Serial.print(lensval);
  Serial.println();

  // Respond to the client
  server.send(200, "application/json", "{}");
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
  int lensval = (int)(pow(((float)(value + lens_x_offset) / (float)pwm_max), 2) * (float)pwm_max);
  ledcWrite(PWM_CHANNEL_Z, lensval);
  Serial.print("Lens (right) Z is set to: ");
  Serial.print(lensval);
  Serial.println();

  // Respond to the client
  server.send(200, "application/json", "{}");
}

/*
 * 
 * 
 *  Set Laser 
 * 
 */

void set_laser() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int value = jsonDocument["value"];

  // apply a nonlinear look-up table
  int laserval = (int)(pow(((float)(value) / (float)pwm_max), 2) * (float)pwm_max);
  ledcWrite(PWM_CHANNEL_LASER, laserval );
  Serial.print("Laser is set to: ");
  Serial.print(laserval);
  Serial.println();

  // Respond to the client
  server.send(200, "application/json", "{}");
}




/*

   Move Motors

*/

void move_x() {
  
  Serial.println("move_X");
  
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int steps = jsonDocument["steps"];
  int speed = jsonDocument["speed"];
  
  // Set the speed to 5 rpm:
  stepper_x.setSpeed(speed);

  Serial.print("steps x: ");
  Serial.print(steps);
  stepper_x.step(steps );

  // Respond to the client
  server.send(200, "application/json", "{}");
  
}




void move_y() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int steps = jsonDocument["steps"];
  int speed = jsonDocument["speed"];

  // Set the speed to 5 rpm:
  stepper_y.setSpeed(speed);

  Serial.print("steps y: ");
  Serial.print(steps);
  stepper_y.step(steps );

  // Respond to the client
  server.send(200, "application/json", "{}");
}





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
  Serial.print("Red: ");
  Serial.print(red);

  // Respond to the client
  server.send(200, "application/json", "{}");
}
