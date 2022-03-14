/*

   Serial protocol looks like so:

   {"task": "/state_get"}
   returns:

  ++
  {"identifier_name":"UC2_Feather","identifier_id":"V0.1","identifier_date":"2022-02-04","identifier_author":"BD"}
  --

  {"task": "/state_set", "isdebug":0}



  turn on the laser:
  {"task": "/laser_act", "LASERid":1, "LASERval":10000, "LASERdespeckle":100}

  move the motor
  {"task": "/motor_act", "speed":1000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isblock":1, "isen":1}
  {"task": "/motor_act", "speed":1000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isblock":0, "isen":1} // move in the background
  {"task": "/motor_act", "speed1":1000,"speed2":100,"speed3":5000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isblock":0, "isen":1}
  {"task": "/motor_act", "isstop":1}
  {'task': '/motor_set', 'axis': 1, 'currentposition': 1}
  {'task': '/motor_set', 'axis': 1, 'sign': 1} // 1 or -1
  {'task': '/motor_set', 'axis': 1, 'sign': 1} // 1 or -1
  {'task': '/motor_get', 'axis': 1}


  operate the analog out
  {"task": "/analog_act", "analogid": 1, "analogval":1000}

 operate the digital out
  {"task": "/digital_act", "digitalid": 1, "digitalval":1}

  operate the dac (e.g. lightsheet)
  {"task": "/dac_act", "dac_channel": 1, "frequency":1, "offset":0, "amplitude":0, "clk_div": 1000}


  // attempt to have fast triggering

  We want to send a table of tasks:

  move motor, wait, take picture cam 1/2, wait, move motor..
  {
  "task": "multitable",
  "task_n": 9,
  "repeats_n": 5,
  "0": {"task": "/motor_act", "speed":1000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isblock":1, "isen":1},
  "1": {"task": "/state_act", "delay": 1000},
  "2": {"task": "/digital_act", "digitalid": 1, "digitalval":1},
  "3": {"task": "/digital_act", "digitalid": 1, "digitalval":0},
  "4": {"task": "/digital_act", "digitalid": 2, "digitalval":1},
  "5": {"task": "/digital_act", "digitalid": 2, "digitalval":0},
  "6": {"task": "/laser_act", "LASERid":1, "LASERval":10000, "LASERdespeckle":100},
  "7": {"task": "/state_act", "delay": 1000},
  "8": {"task": "/laser_act", "LASERid":1, "LASERval":10000, "LASERdespeckle":100}
  }


*/



/*
    Pindefintion per Setup
*/
//#include "pindef_lightsheet.h"
//#include "pindef_lightsheet_arduino.h"
#include "pindef_lightsheet_esp_tomo.h"
//#include "pindef_ptychography.h"
//#include "pindef.h"
//#include "pindef_multicolour.h"
//#include "pindef_STORM_Berlin.h"
//#include "pindef_cellSTORM_cellphone.h"
//#include "pindef_cellSTORM.h"
//#include "pindef_cellSTORM_wifi.h"
//#include "pindef_multicolour_borstel.h"

#ifdef IS_ARDUINO
int DEBUG = 0; // if tihs is set to true, the arduino runs into problems during multiple serial prints..
#else
int DEBUG = 1;
#endif
#define BAUDRATE 115200

/*
    IMPORTANT: ALL setup-specific settings can be found in the "pindef.h" files
*/

// For PS4 support, please install this library https://github.com/beniroquai/PS4-esp32/

#ifdef IS_WIFI
#include <WiFi.h>
#include <WebServer.h>
#include "parameters_wifi.h"
#endif

#ifdef IS_ANALOG
#include "parameters_analog.h"
#endif
#ifdef IS_DIGITAL
#include "parameters_digital.h"
#endif

#include <ArduinoJson.h>

//Where the JSON for the current instruction lives
#ifdef IS_ARDUINO
// shhould not be more than 300 !!!
//StaticJsonDocument<300> jsonDocument;
//char* content = malloc(300);
DynamicJsonDocument jsonDocument(256);
//StaticJsonDocument<256> jsonDocument;
#else
DynamicJsonDocument jsonDocument(2048);
#endif

#ifdef IS_WIFI
WebServer server(80);
char output[1000];
#endif

#ifdef IS_DAC
#include "DAC_Module.h"
DAC_Module *dac = new DAC_Module();
#endif

#ifdef IS_PS3
#include <Ps3Controller.h>
#endif

#ifdef IS_PS4
#include <PS4Controller.h>
#endif
/*
   Register devices
*/
#ifdef IS_MOTOR
#include "A4988.h"
#include "parameters_motor.h"

A4988 stepper_X(FULLSTEPS_PER_REV_X, DIR_X, STEP_X, SLEEP, MS1, MS2, MS3);
A4988 stepper_Y(FULLSTEPS_PER_REV_Y, DIR_Y, STEP_Y, SLEEP, MS1, MS2, MS3);
A4988 stepper_Z(FULLSTEPS_PER_REV_Z, DIR_Z, STEP_Z, SLEEP, MS1, MS2, MS3);
#endif

#ifdef IS_LASER
#include "parameters_laser.h"
#endif

/*
   Register functions
*/



const char* state_act_endpoint = "/state_act";
const char* state_set_endpoint = "/state_set";
const char* state_get_endpoint = "/state_get";

#ifdef IS_LASER
const char* laser_act_endpoint = "/laser_act";
const char* laser_set_endpoint = "/laser_set";
const char* laser_get_endpoint = "/laser_get";
#endif

#ifdef IS_MOTOR
const char* motor_act_endpoint = "/motor_act";
const char* motor_set_endpoint = "/motor_set";
const char* motor_get_endpoint = "/motor_get";
#endif

#ifdef IS_DAC
const char* dac_act_endpoint = "/dac_act";
const char* dac_set_endpoint = "/dac_set";
const char* dac_get_endpoint = "/dac_get";
#endif

#ifdef IS_ANALOG
const char* analog_act_endpoint = "/analog_act";
const char* analog_set_endpoint = "/analog_set";
const char* analog_get_endpoint = "/analog_get";
#endif

#ifdef IS_DIGITAL
const char* digital_act_endpoint = "/digital_act";
const char* digital_set_endpoint = "/digital_set";
const char* digital_get_endpoint = "/digital_get";
#endif

#ifdef IS_LASER
const char* ledarr_act_endpoint = "/ledarr_act";
const char* ledarr_set_endpoint = "/ledarr_set";
const char* ledarr_get_endpoint = "/ledarr_get";
#endif


/* --------------------------------------------
   Setup
  --------------------------------------------
*/
void setup()
{
  // Start Serial
  Serial.begin(BAUDRATE);
  Serial.println("Start");
  printInfo();

  jsonDocument.clear();

  // connect to wifi if necessary
#ifdef IS_WIFI
  connectToWiFi();
  setup_routing();
  init_Spiffs();
#endif


  Serial.println(state_act_endpoint);
  Serial.println(state_get_endpoint);
  Serial.println(state_set_endpoint);


#ifdef IS_LEDARR
  setup_matrix();
#endif

#ifdef IS_MOTOR
  setup_motor();
#endif

#ifdef IS_PS3
  Serial.println("Connnecting to the PS3 controller, please please the magic round button in the center..");
  Ps3.attachOnConnect(onConnect);
  Ps3.begin("01:02:03:04:05:06");
  Serial.println("PS3 controler is set up.");
#endif

#ifdef IS_PS4
  Serial.println("Connnecting to the PS4 controller, please please the magic round button in the center..");
  //Ps4.attachOnConnect(onConnectPS4);
  PS4.begin("1a:2b:3c:01:01:01");
  Serial.println("PS4 controler is set up.");
  stepper_X.setSpeedProfile(stepper_X.CONSTANT_SPEED);
  stepper_Y.setSpeedProfile(stepper_Y.CONSTANT_SPEED);
  stepper_Z.setSpeedProfile(stepper_Z.CONSTANT_SPEED);
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

#ifdef IS_ESP32
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
#else
  pinMode(LASER_PIN_1, OUTPUT);
  analogWrite(LASER_PIN_1, 100); delay(500);
  analogWrite(LASER_PIN_1, 0);
  pinMode(LASER_PIN_2, OUTPUT);
  analogWrite(LASER_PIN_2, 100); delay(500);
  analogWrite(LASER_PIN_2, 0);
  pinMode(LASER_PIN_3, OUTPUT);
  analogWrite(LASER_PIN_3, 100); delay(500);
  analogWrite(LASER_PIN_3, 0);

#endif
#endif

#ifdef IS_DAC
  Serial.println("Setting Up DAC");
  dac->Setup(DAC_CHANNEL_1, 0, 1, 0, 0, 2);
  dac->Setup(DAC_CHANNEL_2, 0, 1, 0, 0, 2);
  //delay(1000);
  //dac->Stop(DAC_CHANNEL_1);
#endif


#ifdef IS_ANALOG
  Serial.println("Setting Up analog");
  /* setup the PWM ports and reset them to 0*/
  ledcSetup(PWM_CHANNEL_analog_1, pwm_frequency, pwm_resolution);
  ledcAttachPin(analog_PIN_1, PWM_CHANNEL_analog_1);
  ledcWrite(PWM_CHANNEL_analog_1, 0);

  ledcSetup(PWM_CHANNEL_analog_2, pwm_frequency, pwm_resolution);
  ledcAttachPin(analog_PIN_2, PWM_CHANNEL_analog_2);
  ledcWrite(PWM_CHANNEL_analog_2, 0);
#endif

#ifdef IS_DIGITALanalog
  Serial.println("Setting Up digital");
  /* setup the output nodes and reset them to 0*/
  pinMode(digital_PIN_1, OUTPUT);
  digitalWrite(digital_PIN_1, LOW);

  pinMode(digital_PIN_2, OUTPUT);
  digitalWrite(digital_PIN_2, LOW);

  pinMode(digital_PIN_3, OUTPUT);
  digitalWrite(digital_PIN_3, LOW);

#endif


  // list modules
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
#ifdef IS_PS3
  Serial.println("IS_PS3");
#endif
#ifdef IS_PS4
  Serial.println("IS_PS4");
#endif
#ifdef IS_DAC
  Serial.println(dac_act_endpoint);
  Serial.println(dac_get_endpoint);
  Serial.println(dac_set_endpoint);
#endif
#ifdef IS_MOTOR
  Serial.println(motor_act_endpoint);
  Serial.println(motor_get_endpoint);
  Serial.println(motor_set_endpoint);
#endif
#ifdef IS_LASER
  Serial.println(laser_act_endpoint);
  Serial.println(laser_get_endpoint);
  Serial.println(laser_set_endpoint);
#endif
#ifdef IS_ANALOG
  Serial.println(analog_act_endpoint);
  Serial.println(analog_get_endpoint);
  Serial.println(analog_set_endpoint);
#endif
#ifdef IS_DIGITALGOUT
  Serial.println(digital_act_endpoint);
  Serial.println(digital_get_endpoint);
  Serial.println(digital_set_endpoint);
#endif
#ifdef IS_LEDARR
  Serial.println(ledarr_act_endpoint);
  Serial.println(ledarr_get_endpoint);
  Serial.println(ledarr_set_endpoint);
#endif
}

//char *task = strdup("");
//char* task = "";

void loop() {
#ifdef IS_SERIAL
  if (Serial.available()) {
    DeserializationError error = deserializeJson(jsonDocument, Serial);
    //free(Serial);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    Serial.flush();
    if (DEBUG) serializeJsonPretty(jsonDocument, Serial);

#ifdef IS_ARDUINO
    const char* task = jsonDocument["task"].as<char*>();
    //char* task = jsonDocument["task"];
#else
    String task_s = jsonDocument["task"];
    char task[50];
    task_s.toCharArray(task, 256);
#endif

    //jsonDocument.garbageCollect(); // memory leak?
    /*if (task == "null") return;*/
    if (DEBUG) {
      Serial.print("TASK: ");
      Serial.println(task);
    }


    #ifdef IS_ARDUINO
      jsonProcessor(task);
    #else
    if (strcmp(task, "multitable") == 0) {
      tableProcessor();
    }
    else {
      // Process individual tasks
      jsonProcessor(task);
    }
    #endif

#endif


    // attempting to despeckle by wiggeling the temperature-dependent modes of the laser?
#ifdef IS_LASER
    if (LASER_despeckle_1 > 0 and LASER_val_1 > 0)
      LASER_despeckle(LASER_despeckle_1, 1);
    if (LASER_despeckle_2 > 0 and LASER_val_2 > 0)
      LASER_despeckle(LASER_despeckle_2, 2);
    if (LASER_despeckle_3 > 0 and LASER_val_3 > 0)
      LASER_despeckle(LASER_despeckle_3, 3);
#endif

#ifdef IS_PS3
    control_PS3();
#endif

#ifdef IS_PS4
    control_PS4();
#endif

#ifdef IS_WIFI
    server.handleClient();
#endif


#ifdef IS_MOTOR
    if (not isblock and not isstop) {
      drive_motor_background();
    }
#endif

  }
}


#ifdef IS_ARDUINO
void jsonProcessor(char* task ){
#else
void jsonProcessor(char task[]) {
#endif
  /*
      Return state
  */
  if (strcmp(task, state_act_endpoint) == 0)
    state_act_fct();
  if (strcmp(task, state_set_endpoint) == 0)
    state_set_fct();
  if (strcmp(task, state_get_endpoint) == 0)
    state_get_fct();

  /*
    Drive Motors
  */
#ifdef IS_MOTOR
  if (strcmp(task, motor_act_endpoint) == 0) {
    motor_act_fct();
  }
  if (strcmp(task, motor_set_endpoint) == 0) {
    motor_set_fct();
  }
  if (strcmp(task, motor_get_endpoint) == 0) {
    motor_get_fct();
  }
#endif

  /*
    Drive DAC
  */
#ifdef IS_DAC
  if (strcmp(task, dac_act_endpoint) == 0)
    dac_act_fct();
  if (strcmp(task, dac_set_endpoint) == 0)
    dac_set_fct();
  if (strcmp(task, dac_get_endpoint) == 0)
    dac_get_fct();
#endif

  /*
    Drive Laser
  */
#ifdef IS_LASER
  if (strcmp(task, laser_act_endpoint) == 0)
    LASER_act_fct();
  if (strcmp(task, laser_set_endpoint) == 0)
    LASER_get_fct();
  if (strcmp(task, laser_get_endpoint) == 0)
    LASER_set_fct();
#endif


  /*
    Drive analog
  */
#ifdef IS_ANALOG
  if (strcmp(task, analog_act_endpoint) == 0)
    analog_act_fct();
  if (strcmp(task, analog_set_endpoint) == 0)
    analog_set_fct();
  if (strcmp(task, analog_get_endpoint) == 0)
    analog_get_fct();
#endif


  /*
    Drive digital
  */
#ifdef IS_DIGITAL
  if (strcmp(task, digital_act_endpoint) == 0)
    digital_act_fct();
  if (strcmp(task, digital_set_endpoint) == 0)
    digital_set_fct();
  if (strcmp(task, digital_get_endpoint) == 0)
    digital_get_fct();
#endif


  /*
    Drive LED Matrix
  */
#ifdef IS_LEDARR
  if (strcmp(task, ledarr_act_endpoint) == 0)
    ledarr_act_fct();
  if (strcmp(task, ledarr_set_endpoint) == 0)
    ledarr_set_fct();
  if (strcmp(task, ledarr_get_endpoint) == 0)
    ledarr_get_fct();
#endif


  // Send JSON information back
  Serial.println("++");
  serializeJson(jsonDocument, Serial);
  Serial.println();
  Serial.println("--");
  jsonDocument.clear();
  jsonDocument.garbageCollect();

}


void tableProcessor() {

  // 1. Copy the table
  DynamicJsonDocument tmpJsonDoc = jsonDocument;
  jsonDocument.clear();

  // 2. now we need to extract the indidvidual tasks
  int N_tasks = tmpJsonDoc["task_n"];
  int N_repeats = tmpJsonDoc["repeats_n"];

  Serial.println("N_tasks");
  Serial.println(N_tasks);
  Serial.println("N_repeats");
  Serial.println(N_repeats);


  for (int irepeats = 0; irepeats < N_repeats; irepeats++){
  for (int itask = 0; itask < N_tasks; itask++) {
  char json_string[256];  
    // Hacky, but should work
    Serial.println(itask);
    serializeJson(tmpJsonDoc[String(itask)], json_string);
    Serial.println(json_string);
    deserializeJson(jsonDocument,json_string);

    String task_s = jsonDocument["task"];
    char task[50];
    task_s.toCharArray(task, 256);

    //jsonDocument.garbageCollect(); // memory leak?
    /*if (task == "null") return;*/
    if (DEBUG) {
      Serial.print("TASK: ");
      Serial.println(task);
    }
    
    jsonProcessor(task);

  }
  }
  tmpJsonDoc.clear();

}
