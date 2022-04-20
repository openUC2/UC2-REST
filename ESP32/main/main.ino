/*

   Serial protocol looks like so:

   {"task": "/state_get"}
   returns:

  ++
  {"identifier_name":"UC2_Feather","identifier_id":"V0.1","identifier_date":"2022-02-04","identifier_author":"BD"}
  --

  {"task": "/state_set", "isdebug":0}

  retrieve sensor value 
  {"task": "/readsensor_act", "readsensorID":0, "N_sensor_avg":100}
  {"task": "/readsensor_get", "readsensorID":0}
  {"task": "/readsensor_set", "readsensorID":0, "readsensorPIN":34, "N_sensor_avg":10}



  turn on the laser:
  {"task": "/laser_act", "LASERid":1, "LASERval":10000, "LASERdespeckle":100}

  move the motor
  {"task": "/motor_act", "speed":1000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isen":1}
  {"task": "/motor_act", "speed":1000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isen":1} // move in the background
  {"task": "/motor_act", "speed1":1000,"speed2":100,"speed3":5000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1,  "isen":1}
  {"task": "/motor_act", "isstop":1}
  {'task': '/motor_set', 'axis': 1, 'currentposition': 1}
  {'task': '/motor_set', 'axis': 1, 'sign': 1} // 1 or -1
  {'task': '/motor_set', 'axis': 1, 'sign': 1} // 1 or -1
  {'task': '/motor_get', 'axis': 1}
  {"task": "/motor_act", "speed":30, "pos1":400, "pos2":0, "pos3":0, "isabs":0, "isblock":0, "isen":1}


  operate the analog out
  {"task": "/analog_act", "analogid": 1, "analogval":1000}

 operate the digital out
  {"task": "/digital_act", "digitalid": 1, "digitalval":1}

  operate the dac (e.g. lightsheet)
  {"task": "/dac_act", "dac_channel": 1, "frequency":1, "offset":0, "amplitude":0, "clk_div": 1000}
amplitude: 0,1,2,3


  operate ledmatrix
  // "pattern", "individual", "full", "off", "left", "right", "top", "bottom",
  {"task": "/ledarr_act","LEDArrMode": "full", "red":100, "green": 100, "blue":100}
  {"task": "/ledarr_act","LEDArrMode": "full", "red":0, "green": 0, "blue":0}
  {'red': 193, 'green': 193, 'blue': 193, 'indexled': 27, 'Nleds': 1, 'LEDArrMode': 'individual', 'task': '/ledarr_act'}

  // attempt to have fast triggering

  We want to send a table of tasks:

  move motor, wait, take picture cam 1/2, wait, move motor..
  {
  "task": "multitable",
  "task_n": 9,
  "repeats_n": 5,
  "0": {"task": "/motor_act", "speed":1000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isblock":1, "isen":1},
  "1": {"task": "/state_act", "delay": 100},
  "2": {"task": "/digital_act", "digitalid": 1, "digitalval":1},
  "3": {"task": "/digital_act", "digitalid": 2, "digitalval":1},
  "4": {"task": "/digital_act", "digitalid": 2, "digitalval":0},
  "5": {"task": "/digital_act", "digitalid": 1, "digitalval":0},
  "6": {"task": "/laser_act", "LASERid":1, "LASERval":10000, "LASERdespeckle":100},
  "7": {"task": "/state_act", "delay": 100},
  "8": {"task": "/laser_act", "LASERid":1, "LASERval":10000, "LASERdespeckle":100}
  }



  // trigger camera at a rate of 20hz

  {"task": "/motor_act", "speed0":0, "speed1":0,"speed2":40,"speed3":9000, "isforever":1, "isaccel":1}
  {"task": "/state_set", "isdebug":0}
  {"task": "/state_act", "delay": 100}
  {
  "task": "multitable",
  "task_n": 2,
  "repeats_n": 200,
  "0": {"task": "/digital_act", "digitalid": 1, "digitalval":-1},
  "1": {"task": "/state_act", "delay": 50}
  }
  {"task": "/motor_act", "isstop":1}
  {"task": "/motor_act", "isenable":0}
  {"task": "/s-tate_set", "isdebug":1}

*/



/*
    Pindefintion per Setup
*/

//#include "pindef_lightsheet.h"
//#include "pindef_lightsheet_arduino.h"
//#include "pindef_ptychography.h"
//#include "pindef.h"
//#include "pindef_multicolour.h"
//#include "pindef_STORM_Berlin.h"
//#include "pindef_cellSTORM_cellphone.h"
//#include "pindef_cellSTORM.h"
//#include "pindef_cellSTORM_wifi.h"
//#include "pindef_multicolour_borstel.h"
//#include "pindef_cncshield_esp.h"
//#include "pindef_lightsheet_tomo_galvo.h"
#include "pindef_lightsheet_tomo_galvo_espwemos.h"
//#include "pindef_lightsheet_espwemos.h"


int DEBUG = 1; // if tihs is set to true, the arduino runs into problems during multiple serial prints..
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

#ifdef IS_READSENSOR
//#include "parameters_readsensor.h"

#endif

#include <ArduinoJson.h>
#include "parameters_state.h"
#if defined(IS_DAC) || defined(IS_DAC_FAKE)
#include "parameters_dac.h"
uint32_t frequency = 1000;
#endif

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

// ensure motors switch off when PS3 controller is operating them
bool override_overheating = false;


#ifdef IS_DAC
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
#include <AccelStepper.h>
#include "parameters_motor.h"
//AccelStepper stepper_A = AccelStepper(AccelStepper::DRIVER, STEP_A, DIR_A);
AccelStepper stepper_X = AccelStepper(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepper_Y = AccelStepper(AccelStepper::DRIVER, STEP_Y, DIR_Y);
AccelStepper stepper_Z = AccelStepper(AccelStepper::DRIVER, STEP_Z, DIR_Z);
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

#ifdef IS_LEDARR
const char* ledarr_act_endpoint = "/ledarr_act";
const char* ledarr_set_endpoint = "/ledarr_set";
const char* ledarr_get_endpoint = "/ledarr_get";
#endif


#ifdef IS_READSENSOR
const char* readsensor_act_endpoint = "/readsensor_act";
const char* readsensor_set_endpoint = "/readsensor_set";
const char* readsensor_get_endpoint = "/readsensor_get";
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
  Ps3.attach(onAttach);
  Ps3.attachOnConnect(onConnect);
  Ps3.attachOnDisconnect(onDisConnect);
  Ps3.begin("01:02:03:04:05:06");
  Serial.println("01:02:03:04:05:06");
  //String address = Ps3.getAddress(); // have arbitrary address?
  //Serial.println(address);
  Serial.println("PS3 controler is set up.");
#endif

#ifdef IS_PS4
  Serial.println("Connnecting to the PS4 controller, please please the magic round button in the center..");
  //Ps4.attachOnConnect(onConnectPS4);
  PS4.begin("1a:2b:3c:01:01:01");
  Serial.println("PS4 controler is set up.");
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
  //Setup(dac_channel, clk_div, frequency, scale, phase, invert);
  dac->Setup(DAC_CHANNEL_1, 1000, 50, 0, 0, 2);
  dac->Setup(DAC_CHANNEL_2, 1000, 50, 0, 0, 2);
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

#ifdef IS_DIGITAL
  setupDigital();
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


#ifdef IS_DAC_FAKE
  pinMode(dac_fake_1, OUTPUT);
  pinMode(dac_fake_2, OUTPUT);
  frequency=1;
  xTaskCreate(
    drive_galvo,    // Function that should be called
    "drive_galvo",   // Name of the task (for debugging)
    1000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
#endif


#ifdef IS_READSENSOR
setup_sensors();
Serial.println(readsensor_act_endpoint);
Serial.println(readsensor_set_endpoint);
Serial.println(readsensor_get_endpoint);
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

    // do the processing based on the incoming stream
    if (strcmp(task, "multitable") == 0) {
      // multiple tasks
      tableProcessor();
    }
    else {
      // Process individual tasks
      jsonProcessor(task);
    }

  }
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
   control_PS3(); // if controller is operating motors, overheating protection is enabled
#endif

#ifdef IS_PS4
    control_PS4();
#endif

#ifdef IS_WIFI
    server.handleClient();
#endif


#ifdef IS_MOTOR
    if (not isstop) {
      isactive=true;
      drive_motor_background();
    }
#endif


}

void jsonProcessor(char task[]) {


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


  /*
  Read the sensor
  */
 #ifdef IS_READSENSOR
    if (strcmp(task, readsensor_act_endpoint) == 0)
    readsensor_act_fct();
  if (strcmp(task, readsensor_set_endpoint) == 0)
    readsensor_set_fct();
  if (strcmp(task, readsensor_get_endpoint) == 0)
    readsensor_get_fct();
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
