/*
  This a simple example of the aREST Library for Arduino (Uno/Mega/Due/Teensy)
  using the Serial port. See the README file for more details.

  Written in 2014 by Marco Schwartz under a GPL license.
*/


#define DEBUG 1

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
#define IS_ARDUINO
#else
#define IS_ESP32
#include <WiFi.h>
#include <WebServer.h>
#include "wifi_parameters.h"
#endif


#include <ArduinoJson.h>
#include <AccelStepper.h>


#include "def_motor.h"





//Where the JSON for the current instruction lives
#ifdef IS_ESP32
char buffer[2500];
DynamicJsonDocument jsonDocument(2048);
WebServer server(80);
String output;
#else
DynamicJsonDocument jsonDocument(200);
#endif


/*
 * 
 * Register devices
 * 
 */

// https://www.pjrc.com/teensy/td_libs_AccelStepper.html
AccelStepper stepper_X = AccelStepper(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepper_Y = AccelStepper(AccelStepper::DRIVER, STEP_Y, DIR_Y);
AccelStepper stepper_Z = AccelStepper(AccelStepper::DRIVER, STEP_Z, DIR_Z);



/*
 * 
 * Register functions
 * 
 */
String motor_act_endpoint = "/motor_act";
String motor_set_endpoint = "/motor_set";
String motor_get_endpoint = "/motor_get";

void setup(void)
{
  // Start Serial
  Serial.begin(115200);
  Serial.println("Start");

  // connect to wifi if necessary
#ifdef IS_ESP32
  connectToWiFi();
  setup_routing();
#endif


  /*
     Motor related settings
  */  
  Serial.println("Setting Up Motors");
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);

  stepper_X.setMaxSpeed(MAX_VELOCITY_X_mm * steps_per_mm_X);
  stepper_Y.setMaxSpeed(MAX_VELOCITY_Y_mm * steps_per_mm_Y);
  stepper_Z.setMaxSpeed(MAX_VELOCITY_Z_mm * steps_per_mm_Z);

  stepper_X.setAcceleration(MAX_ACCELERATION_X_mm * steps_per_mm_X);
  stepper_Y.setAcceleration(MAX_ACCELERATION_Y_mm * steps_per_mm_Y);
  stepper_Z.setAcceleration(MAX_ACCELERATION_Z_mm * steps_per_mm_Z);

  stepper_X.enableOutputs();
  stepper_Y.enableOutputs();
  stepper_Z.enableOutputs();

}




void loop() {
#ifdef IS_ARDUINO
  if (Serial.available()) {
    deserializeJson(jsonDocument, Serial);
    String task = jsonDocument["task"];
    if (task == motor_act_endpoint)
      motor_act_fct(jsonDocument);
    else if (task == motor_set_endpoint)
      motor_set_fct(jsonDocument);
    else if (task == motor_get_endpoint)
      jsonDocument = motor_get_fct(jsonDocument);

    //Senden
    serializeJson(jsonDocument, Serial);
    Serial.println();
  }
#endif


#ifdef IS_ESP32
  server.handleClient();
#endif

}




/*
 * 
 * 
 * Define Endpoints
 * 
 * 
 */

#ifdef IS_ESP32
void setup_routing() {
  // GET
  //  server.on("/temperature", getTemperature);
  //server.on("/env", getEnv);
  // https://www.survivingwithandroid.com/esp32-rest-api-esp32-api-server/

  // POST
  server.on(motor_act_endpoint, HTTP_POST, motor_act_fct_http);
  server.on(motor_get_endpoint, HTTP_POST, motor_get_fct_http);
  server.on(motor_set_endpoint, HTTP_POST, motor_set_fct_http);

  // start server
  server.begin();
}
#endif
