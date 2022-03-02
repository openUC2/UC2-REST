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
  {"task": "/motor_act", "isstop":1}  
  {'task': '/motor_set', 'axis': 1, 'currentposition': 1}
  {'task': '/motor_set', 'axis': 1, 'sign': 1} // 1 or -1
  {'task': '/motor_set', 'axis': 1, 'sign': 1} // 1 or -1
  {'task': '/motor_get', 'axis': 1}
  

  operate the analog out
  {"task": "/analogout_act", "analogoutid": 1, "analogoutval":1000}

  operate the dac (e.g. lightsheet)
  {"task": "/dac_act", "dac_channel": 1, "frequency":1, "offset":0, "amplitude":0, "clk_div": 1000}

*/
/*
    Pindefintion per Setup
*/
//#include "pindef_lightsheet.h"
#include "pindef_lightsheet_arduino.h"
//#include "pindef_ptychography.h"
//#include "pindef.h"
//#include "pindef_multicolour.h"
//#include "pindef_STORM_Berlin.h"
//#include "pindef_cellSTORM_cellphone.h"
//#include "pindef_cellSTORM.h"
//#include "pindef_multicolour_borstel.h"



int DEBUG = 1; // if tihs is set to true, the arduino runs into problems during multiple serial prints..
#define BAUDRATE 115200

/*
    IMPORTANT: ALL setup-specific settings can be found in the "pindef.h" files
*/

// For PS4 support, please install this library https://github.com/beniroquai/PS4-esp32/

#ifdef IS_WIFI
#include <WiFi.h>
#include <WebServer.h>
#include "wifi_parameters.h"
#endif

#ifdef IS_ANALOGOUT
#include "analogout_parameters.h"
#endif



#include <ArduinoJson.h>

//Where the JSON for the current instruction lives
#ifdef IS_ARDUINO
// shhould not be more than 300 !!!
//StaticJsonDocument<300> jsonDocument;
//char* content = malloc(300);
//DynamicJsonDocument jsonDocument(256);
StaticJsonDocument<256> jsonDocument;
#else
//char buffer[2500];
DynamicJsonDocument jsonDocument(2048);
#endif
String output;


#ifdef IS_WIFI
WebServer server(80);
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
#include "SyncDriver.h"
#include "motor_parameters.h"

A4988 stepper_X(FULLSTEPS_PER_REV_X, DIR_X, STEP_X, SLEEP, MS1, MS2, MS3);
A4988 stepper_Y(FULLSTEPS_PER_REV_Y, DIR_Y, STEP_Y, SLEEP, MS1, MS2, MS3);
A4988 stepper_Z(FULLSTEPS_PER_REV_Z, DIR_Z, STEP_Z, SLEEP, MS1, MS2, MS3);
//SyncDriver controller(stepper_X, stepper_Y, stepper_Z);
#endif

#ifdef IS_LASER
#include "LASER_parameters.h"
#endif

/*
   Register functions
*/
const char* laser_act_endpoint = "/laser_act";
const char* laser_set_endpoint = "/laser_set";
const char* laser_get_endpoint = "/laser_get";
const char* motor_act_endpoint = "/motor_act";
const char* motor_set_endpoint = "/motor_set";
const char* motor_get_endpoint = "/motor_get";
const char* dac_act_endpoint = "/dac_act";
const char* dac_set_endpoint = "/dac_set";
const char* dac_get_endpoint = "/dac_get";
const char* state_act_endpoint = "/state_act";
const char* state_set_endpoint = "/state_set";
const char* state_get_endpoint = "/state_get";
const char* analogout_act_endpoint = "/analogout_act";
const char* analogout_set_endpoint = "/analogout_set";
const char* analogout_get_endpoint = "/analogout_get";
const char* ledarr_act_endpoint = "/ledarr_act";
const char* ledarr_set_endpoint = "/ledarr_set";
const char* ledarr_get_endpoint = "/ledarr_get";



/*
   Setup
*/
void setup(void)
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


#ifdef IS_ANALOGOUT
  Serial.println("Setting Up ANALOGOUT");
  /* setup the PWM ports and reset them to 0*/
  ledcSetup(PWM_CHANNEL_analogout_1, pwm_frequency, pwm_resolution);
  ledcAttachPin(analogout_PIN_1, PWM_CHANNEL_analogout_1);
  ledcWrite(PWM_CHANNEL_analogout_1, 0);

  ledcSetup(PWM_CHANNEL_analogout_2, pwm_frequency, pwm_resolution);
  ledcAttachPin(analogout_PIN_2, PWM_CHANNEL_analogout_2);
  ledcWrite(PWM_CHANNEL_analogout_2, 0);
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
#ifdef IS_ANALOGOUT
  Serial.println(analogout_act_endpoint);
  Serial.println(analogout_get_endpoint);
  Serial.println(analogout_set_endpoint);
#endif
#ifdef IS_LEDARR
  Serial.println(ledarr_act_endpoint);
  Serial.println(ledarr_get_endpoint);
  Serial.println(ledarr_set_endpoint);
#endif
}

//char *task = strdup(""); 
char* task = "";

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
    char* task = jsonDocument["task"];
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
      Drive Analogout
    */
#ifdef IS_ANALOGOUT
    if (strcmp(task, analogout_act_endpoint) == 0)
      analogout_act_fct();
    if (strcmp(task, analogout_set_endpoint) == 0)
      analogout_set_fct();
    if (strcmp(task, analogout_get_endpoint) == 0)
      analogout_get_fct();
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

  freeMemory();

#ifdef IS_MOTOR
  if(not isblock and not isstop){
    drive_motor_background();
  }
#endif
}


/*
   Define Endpoints for HTTP REST API
*/

#ifdef IS_WIFI
void setup_routing() {
  // GET
  //  server.on("/temperature", getTemperature);
  //server.on("/env", getEnv);
  // https://www.survivingwithandroid.com/esp32-rest-api-esp32-api-server/

  server.on(state_act_endpoint, HTTP_POST, state_act_fct_http);
  server.on(state_get_endpoint, HTTP_POST, state_get_fct_http);
  server.on(state_set_endpoint, HTTP_POST, state_set_fct_http);

#ifdef IS_MOTOR
  // POST
  server.on(motor_act_endpoint, HTTP_POST, motor_act_fct_http);
  server.on(motor_get_endpoint, HTTP_POST, motor_get_fct_http);
  server.on(motor_set_endpoint, HTTP_POST, motor_set_fct_http);
#endif

#ifdef IS_DAC
  server.on(dac_act_endpoint, HTTP_POST, dac_act_fct_http);
  server.on(dac_get_endpoint, HTTP_POST, dac_get_fct_http);
  server.on(dac_set_endpoint, HTTP_POST, dac_set_fct_http);
#endif

#ifdef IS_LASER
  server.on(laser_act_endpoint, HTTP_POST, LASER_act_fct_http);
  server.on(laser_get_endpoint, HTTP_POST, LASER_get_fct_http);
  server.on(laser_set_endpoint, HTTP_POST, LASER_set_fct_http);
#endif

#ifdef IS_ANALOGOUT
  server.on(analogout_act_endpoint, HTTP_POST, analogout_act_fct_http);
  server.on(analogout_get_endpoint, HTTP_POST, analogout_get_fct_http);
  server.on(analogout_set_endpoint, HTTP_POST, analogout_set_fct_http);
#endif

#ifdef IS_LEDARR
  server.on(ledarr_act_endpoint, HTTP_POST, ledarr_act_fct_http);
  server.on(ledarr_get_endpoint, HTTP_POST, ledarr_get_fct_http);
  server.on(ledarr_set_endpoint, HTTP_POST, ledarr_set_fct_http);
#endif




  // start server
  server.begin();
}
#endif





#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 & ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
