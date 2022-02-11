#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Stepper.h>



// Setup MOTOR
// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 120

#define DIR 4
#define STEP 2
#define SLEEP 13 // optional (just delete SLEEP from everywhere if not used)
#include "A4988.h"
#define MS1 10
#define MS2 11
#define MS3 12
A4988 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MS1, MS2, MS3);


//#define home_wifi
#ifdef home_wifi
  const char *ssid = "BenMur";
  const char *password = "MurBen3128";  
#else
  const char* ssid = "Blynk";
  const char* password = "12345678";  
#endif


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

int glob_motor_steps = 0;

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

  /* set led and laser as output to control led on-off */
  pinMode(LED_PIN, OUTPUT);

  // switch of the laser directly
  pinMode(LASER_PIN_PLUS, OUTPUT);
  pinMode(LASER_PIN_MINUS, OUTPUT);
  digitalWrite(LASER_PIN_PLUS, LOW);
  digitalWrite(LASER_PIN_MINUS, LOW);

  // initiliaze connection
  connectToWiFi();
  setup_routing();

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


  /*
   * Set target motor RPM.
   */
  stepper.begin(RPM);
  // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
  // stepper.setEnableActiveState(LOW);
  stepper.enable();

  stepper.setMicrostep(1);  // Set microstep mode to 1:1
  
  // One complete revolution is 360°
  stepper.rotate(360);     // forward revolution
  stepper.rotate(-360);    // reverse revolution

  // assign a task for the motor so that it runs when we need it
  xTaskCreate(
                    runStepper,             /* Task function. */
                    "runStepper",           /* String with name of task. */
                    10000,                     /* Stack size in words. */
                    (void*)&glob_motor_steps,      /* Parameter passed as input of the task */
                    1,                         /* Priority of the task. */
                    NULL);                     /* Task handle. */
 
}

void loop() {
  server.handleClient();

  
}

// the motor kinda always runs but with the numbers we provide it with
void runStepper( void * parameter){
  while(true)
    stepper.move(*((int*)parameter));
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
  server.on("/led", HTTP_POST, set_led);
  server.on("/move_z", HTTP_POST, move_z);
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
  int laserval = value; 
  //int laserval = (int)(pow(((float)(value) / (float)pwm_max), 2) * (float)pwm_max);
  // Respond to the client
  server.send(200, "application/json", "{}");

  // PErform action
  ledcWrite(PWM_CHANNEL_LASER, laserval );
  Serial.print("Laser is set to: ");
  Serial.print(laserval);
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
  stepper.begin(abs(speed));

  if(steps==0){
    // run unsupervised
    glob_motor_steps=sgn(speed)*10;
  }
  else{
    // run only the number of steps we want
    glob_motor_steps=0;
    stepper.move(steps);
  }
    
  Serial.print("steps x: ");
  Serial.print(glob_motor_steps);
  //stepper.move(speed);

  // Respond to the client

  
}


static inline int8_t sgn(int val) {
  if (val < 0) return -1;
  if (val == 0) return 0;
  return 1;
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
