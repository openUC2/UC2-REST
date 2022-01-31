#define DEBUG 1
// CASES:
// 1 Arduino -> Serial only
// 2 ESP32 -> Serial only
// 3 ESP32 -> Wifi only
// 4 ESP32 -> Wifi + Serial ?

// load configuration
//#define ARDUINO_SERIAL
#define ESP32_SERIAL
//#define ESP32_WIFI
//#define ESP32_SERIAL_WIFI

// load modules
//#define IS_DAC 1 // ESP32-only
#define IS_LASER // ESP32-only
#define IS_MOTOR

#ifdef ARDUINO_SERIAL
#define IS_SERIAL
#define IS_ARDUINO
#endif

#ifdef ESP32_SERIAL
#define IS_SERIAL
#define IS_ESP32
#endif

#ifdef ESP32_WIFI
#define IS_WIFI
#define IS_ESP32
#endif

#ifdef ESP32_SERIAL_WIFI
#define IS_WIFI
#define IS_SERIAL
#define IS_ESP32
#endif



/*
   START CODE HERE
*/


#ifdef IS_WIFI
#include <WiFi.h>
#include <WebServer.h>
#include "wifi_parameters.h"
#endif


#include <ArduinoJson.h>
//#include <AccelStepper.h>
#include "A4988.h"
#include "motor_parameters.h"
#include "LASER_parameters.h"



//Where the JSON for the current instruction lives
#ifdef IS_ARDUINO
DynamicJsonDocument jsonDocument(400);
#else
char buffer[2500];
DynamicJsonDocument jsonDocument(2048);
#endif
String output;


#ifdef IS_WIFI && IS_ESP32
WebServer server(80);
#endif

#ifdef IS_DAC
#include "DAC_Module.h"
DAC_Module *dac = new DAC_Module();
#endif


/*

   Register devices

*/
#ifdef IS_MOTOR
// https://www.pjrc.com/teensy/td_libs_AccelStepper.html
A4988 stepper_X(FULLSTEPS_PER_REV_X, DIR_X, STEP_X, SLEEP, MS1, MS2, MS3);
A4988 stepper_Y(FULLSTEPS_PER_REV_Y, DIR_Y, STEP_Y, SLEEP, MS1, MS2, MS3);
A4988 stepper_Z(FULLSTEPS_PER_REV_Z, DIR_Z, STEP_Z, SLEEP, MS1, MS2, MS3);
//AccelStepper stepper_X = AccelStepper(AccelStepper::DRIVER, STEP_X, DIR_X);
//AccelStepper stepper_Y = AccelStepper(AccelStepper::DRIVER, STEP_Y, DIR_Y);
//AccelStepper stepper_Z = AccelStepper(AccelStepper::DRIVER, STEP_Z, DIR_Z);
#endif

/*
   Register functions
*/
String motor_act_endpoint = "/motor_act";
String motor_set_endpoint = "/motor_set";
String motor_get_endpoint = "/motor_get";
String DAC_act_endpoint = "/DAC_act";
String DAC_set_endpoint = "/DAC_set";
String DAC_get_endpoint = "/DAC_get";
String LASER_act_endpoint = "/LASER_act";
String LASER_set_endpoint = "/LASER_set";
String LASER_get_endpoint = "/LASER_get";

/*
   Setup
*/
void setup(void)
{
  // Start Serial
  Serial.begin(115200);
  Serial.println("Start");

  // connect to wifi if necessary
#ifdef IS_WIFI && IS_ESP32
  connectToWiFi();
  setup_routing();
#endif


#ifdef IS_MOTOR
  /*
     Motor related settings
  */
  Serial.println("Setting Up Motors");
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);

  stepper_X.begin(RPM);
  stepper_X.enable();
  stepper_X.setMicrostep(1);
  stepper_X.move(100);
  stepper_X.move(-100);

  stepper_Y.begin(RPM);
  stepper_Y.enable();
  stepper_Y.setMicrostep(1);
  stepper_Y.move(100);
  stepper_Y.move(-100);

  stepper_Z.begin(RPM);
  stepper_Z.enable();
  stepper_Z.setMicrostep(1);
  stepper_Z.move(100);
  stepper_Z.move(-100);

  /*
    stepper_X.setMaxSpeed(MAX_VELOCITY_X_mm * steps_per_mm_X);
    stepper_Y.setMaxSpeed(MAX_VELOCITY_Y_mm * steps_per_mm_Y);
    stepper_Z.setMaxSpeed(MAX_VELOCITY_Z_mm * steps_per_mm_Z);

    stepper_X.setAcceleration(MAX_ACCELERATION_X_mm * steps_per_mm_X);
    stepper_Y.setAcceleration(MAX_ACCELERATION_Y_mm * steps_per_mm_Y);
    stepper_Z.setAcceleration(MAX_ACCELERATION_Z_mm * steps_per_mm_Z);

    stepper_X.enableOutputs();
    stepper_Y.enableOutputs();
    stepper_Z.enableOutputs();
  */
#endif


#ifdef IS_LASER
  Serial.println("Setting Up LASERs");
  // switch of the LASER directly
  pinMode(LASER_PIN_1, OUTPUT);
  pinMode(LASER_PIN_2, OUTPUT);
  pinMode(LASER_PIN_3, OUTPUT);
  digitalWrite(LASER_PIN_1, LOW);
  digitalWrite(LASER_PIN_2, LOW);
  digitalWrite(LASER_PIN_3, LOW);
  /* setup the PWM ports and reset them to 0*/
  ledcSetup(PWM_CHANNEL_LASER_1, pwm_frequency, pwm_resolution);
  ledcAttachPin(LASER_PIN_1, PWM_CHANNEL_LASER_1);
  ledcWrite(PWM_CHANNEL_LASER_1, 10000); delay(500);
  ledcWrite(PWM_CHANNEL_LASER_1, 0);
  
  ledcSetup(PWM_CHANNEL_LASER_2, pwm_frequency, pwm_resolution);
  ledcAttachPin(LASER_PIN_2, PWM_CHANNEL_LASER_2);
  ledcWrite(PWM_CHANNEL_LASER_2, 10000); delay(500);
  ledcWrite(PWM_CHANNEL_LASER_2, 0);

  ledcSetup(PWM_CHANNEL_LASER_3, pwm_frequency, pwm_resolution);
  ledcAttachPin(LASER_PIN_3, PWM_CHANNEL_LASER_3);
  ledcWrite(PWM_CHANNEL_LASER_3, 10000); delay(500);
  ledcWrite(PWM_CHANNEL_LASER_3, 0);
#endif

#ifdef IS_DAC
  Serial.println("Setting Up DAC");
  dac->Setup(DAC_CHANNEL_1, 0, 1000, 0, 0, 2);
  delay(1000);
  dac->Stop(DAC_CHANNEL_1);
#endif


  // list modules
#ifdef IS_LASER
  Serial.println("IS_LASER");
#endif
#ifdef IS_DAC
  Serial.println("IS_DAC");
#endif
#ifdef IS_SERIAL
  Serial.println("IS_SERIAL");
#endif
#ifdef IS_WIFI
  Serial.println("IS_WIFI");
#endif
#ifdef IS_ARDUINO
  Serial.println("IS_ARDUINO");
#endif
#ifdef IS_ESP32
  Serial.println("IS_ESP32");
#endif
#ifdef IS_MOTOR
  Serial.println("IS_MOTOR");
#endif

}




void loop() {
#ifdef IS_SERIAL
  if (Serial.available()) {
    deserializeJson(jsonDocument, Serial);
    String task = jsonDocument["task"];

    if (task == "null") return;
    if (DEBUG) Serial.print("TASK: "); Serial.println(task);

#ifdef IS_MOTOR
    if (task == motor_act_endpoint) {
      motor_act_fct(jsonDocument);
    }
    else if (task == motor_set_endpoint)
      motor_set_fct(jsonDocument);
    else if (task == motor_get_endpoint)
      jsonDocument = motor_get_fct(jsonDocument);
#endif

#ifdef IS_DAC
    else if (task == DAC_act_endpoint)
      DAC_act_fct(jsonDocument);
    else if (task == DAC_set_endpoint)
      DAC_set_fct(jsonDocument);
    else if (task == DAC_get_endpoint)
      jsonDocument = DAC_get_fct(jsonDocument);
#endif


#ifdef IS_LASER
    else if (task == LASER_act_endpoint)
      jsonDocument = LASER_act_fct(jsonDocument);
    else if (task == LASER_set_endpoint)
      jsonDocument = LASER_get_fct(jsonDocument);
    else if (task == LASER_get_endpoint)
      jsonDocument = LASER_set_fct(jsonDocument);
#endif

    // Send JSON information back
    serializeJson(jsonDocument, Serial);
    Serial.println();
    Serial.println("//");


  }
#endif

  /*
    stepper_X.run();
    stepper_Y.run();
    stepper_Z.run();
  */
#ifdef IS_WIFI && IS_ESP32
  server.handleClient();
#endif

}


/*


   Define Endpoints


*/

#ifdef IS_WIFI && IS_ESP32
void setup_routing() {
  // GET
  //  server.on("/temperature", getTemperature);
  //server.on("/env", getEnv);
  // https://www.survivingwithandroid.com/esp32-rest-api-esp32-api-server/

#ifdef IS_MOTOR
  // POST
  server.on(motor_act_endpoint, HTTP_POST, motor_act_fct_http);
  server.on(motor_get_endpoint, HTTP_POST, motor_get_fct_http);
  server.on(motor_set_endpoint, HTTP_POST, motor_set_fct_http);
#endif

#ifdef IS_DAC
  server.on(DAC_act_endpoint, HTTP_POST, DAC_act_fct_http);
  server.on(DAC_get_endpoint, HTTP_POST, DAC_get_fct_http);
  server.on(DAC_set_endpoint, HTTP_POST, DAC_set_fct_http);
#endif

#ifdef IS_LASER
  server.on(LASER_act_endpoint, HTTP_POST, LASER_act_fct_http);
  server.on(LASER_get_endpoint, HTTP_POST, LASER_get_fct_http);
  server.on(LASER_set_endpoint, HTTP_POST, LASER_set_fct_http);
#endif

  // start server
  server.begin();
}
#endif
