#include "config.h"

#include "src/motor/FocusMotor.h"
#ifdef IS_LED
  #include "src/led/led_controller.h"
#endif
#ifdef IS_LASER
  #include "src/laser/LaserController.h"
#endif
#ifdef IS_ANALOG
  #include "src/analog/AnalogController.h"
#endif
#include "src/state/State.h"
#ifdef IS_SCANNER
  #include "src/scanner/ScannerController.h"
#endif
#ifdef IS_PID
  #include "src/pid/PidController.h"
#endif
#ifdef IS_DIGITAL
  #include "src/digital/DigitalController.h"
  #endif
#ifdef IS_READSENSOR
  #include "src/sensor/SensorController.h"
#endif

#ifdef IS_DAC || IS_DAC_FAKE
  #include "src/dac/DacController.h"
#endif
#ifdef IS_PS3
  #include "src/gamepads/ps3_controller.h"
#endif
#ifdef IS_PS4
  #include "src/gamepads/ps4_controller.h"
#endif
#include "src/wifi/WifiController.h"

#include <ArduinoJson.h>

/*
    IMPORTANT: ALL setup-specific settings can be found in the "pindef.h" files
*/

// For PS4 support, please install this library https://github.com/beniroquai/PS4-esp32/
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


//Where the JSON for the current instruction lives
#ifdef IS_SLM
  DynamicJsonDocument jsonDocument(32784);
#else
  DynamicJsonDocument jsonDocument(4096);
#endif


// ensure motors switch off when PS3 controller is operating them
bool override_overheating = false;

#ifdef IS_SLM
#include "parameters_slm.h"
#endif

PINDEF * pins;
/* --------------------------------------------
   Setup
  --------------------------------------------
*/
void setup()
{
  /*
     SETTING UP DEVICES
  */

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector 

  // for any timing related puposes..
  state->startMillis = millis();
  pins = new PINDEF();
  getDefaultPinDef((*pins));
  // Start Serial
  Serial.begin(BAUDRATE);
  Serial.println("Start");
  state->printInfo();

  jsonDocument.clear();

  // connect to wifi if necessary
#ifdef IS_WIFI
  bool isResetWifiSettings = false;
  wifi = new WifiController();
  wifi->autoconnectWifi(isResetWifiSettings);
  wifi->setup_routing();
  wifi->init_Spiffs();
#endif
  Serial.println(state_act_endpoint);
  Serial.println(state_get_endpoint);
  Serial.println(state_set_endpoint);

#ifdef IS_SLM
  setup_slm();
#endif

Serial.println("IS_LEDARR");
led = new led_controller();
#ifdef DEBUG_LED
  led->DEBUG = true;
#endif
led->jsonDocument = &jsonDocument;
led->setup_matrix();
motor = new FocusMotor(pins);
#ifdef DEBUG_MOTOR
  motor->DEBUG = true;
#endif
motor->jsonDocument = &jsonDocument;
motor->setup();

state->clearBlueetoothDevice();
#ifdef IS_PS3
  #ifdef DEBUG_GAMEPAD
    ps3_c->DEBUG = true;
  #endif
  ps3_c->start();
#endif
#ifdef IS_PS4
  #ifdef DEBUG_GAMEPAD
    ps4_c->DEBUG = true;
  #endif
  ps4_c->start();
#endif

#ifdef IS_LASER
  laser->jsonDocument = &jsonDocument;
  laser->setup();
#endif


#if defined IS_DAC || defined IS_DAC_FAKE
  Serial.println("Setting Up DAC");
  dac->jsonDocument = &jsonDocument;
  dac->pins = pins;
  dac->setup();
#endif


#ifdef IS_ANALOG
  #ifdef DEBUG_ANALOG
    analog->DEBUG = true;
  #endif
  analog->jsonDocument = &jsonDocument;
  analog->pins = pins;
  analog->setup();
#endif

#ifdef IS_DIGITAL
  digital->pins = pins;
  digital->setup();
#endif

#ifdef IS_READSENSOR
  sensor->jsonDocument = &jsonDocument;
  sensor->pins = pins;
  sensor->setup();
  Serial.println(readsensor_act_endpoint);
  Serial.println(readsensor_set_endpoint);
  Serial.println(readsensor_get_endpoint);
#endif

#ifdef IS_PID
  pid->jsonDocument = &jsonDocument;
  pid->pins = pins;
  pid->setup();
  Serial.println(PID_act_endpoint);
  Serial.println(PID_set_endpoint);
  Serial.println(PID_get_endpoint);
#endif
#ifdef IS_SCANNER
  scanner->pins = pins;
  scanner->setup();
#endif

// list modules
if(wifi != nullptr)
    Serial.println("IS_WIFI");
if(led != nullptr)
    Serial.println("IS_LED");
if(analog != nullptr)
    Serial.println("IS_ANALOG");
if(digital != nullptr)
    Serial.println("IS_DIGITAL");
if(pid != nullptr)
    Serial.println("IS_PID");
if(sensor != nullptr)
    Serial.println("IS_SENSOR");
if(scanner != nullptr)
    Serial.println("IS_SCANNER");
if(dac != nullptr)
{
  #if defined IS_DAC
    Serial.println("IS_DAC");
  #endif
  #if defined IS_DAC_FAKE
    Serial.println("IS_DAC_FAKE");
  #endif
}
if(motor != nullptr)
  Serial.println("IS_MOTOR");
#ifdef IS_SLM
  Serial.println("IS_SLM");
#endif


}

//char *task = strdup("");
//char* task = "";

void loop() {
  // for any timing-related purposes
  state->currentMillis = millis();

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


  String task_s = jsonDocument["task"];
  char task[50];
  task_s.toCharArray(task, 256);


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

  /*
     continous control during loop
  */

  // attempting to despeckle by wiggeling the temperature-dependent modes of the laser?
  if (laser->LASER_despeckle_1 > 0 && laser->LASER_val_1 > 0)
    laser->LASER_despeckle(laser->LASER_despeckle_1, 1, laser->LASER_despeckle_period_1);
  if (laser->LASER_despeckle_2 > 0 && laser->LASER_val_2 > 0)
    laser->LASER_despeckle(laser->LASER_despeckle_2, 2, laser->LASER_despeckle_period_2);
  if (laser->LASER_despeckle_3 > 0 && laser->LASER_val_3 > 0)
    laser->LASER_despeckle(laser->LASER_despeckle_3, 3, laser->LASER_despeckle_period_3);


#if defined IS_PS3
  ps3_c->control(); // if controller is operating motors, overheating protection is enabled
#endif
#if defined IS_PS4
  ps4_c->control(); // if controller is operating motors, overheating protection is enabled
#endif

#ifdef IS_WIFI
  wifi->handelMessages();
#endif

  /*
     continous control during loop
  */
  if (!motor->isstop) {
    motor->isactive = true;
    motor->background();
  }


#ifdef IS_PID
  if (pid->PID_active && (state->currentMillis - state->startMillis >= pid->PID_updaterate)) {
    pid->background();
    state->startMillis = millis();
  }
#endif


}

void jsonProcessor(char task[]) {


  /*
      Return state
  */
  if (strcmp(task, state_act_endpoint) == 0)
    state->act();
  if (strcmp(task, state_set_endpoint) == 0)
    state->set();
  if (strcmp(task, state_get_endpoint) == 0)
    state->get();

  /*
    Drive Motors
  */
  if (strcmp(task, motor_act_endpoint) == 0) {
    motor->act();
  }
  if (strcmp(task, motor_set_endpoint) == 0) {
    motor->set();
  }
  if (strcmp(task, motor_get_endpoint) == 0) {
    motor->get();
  }


  /*
    Operate SLM
  */
#ifdef IS_SLM
  if (strcmp(task, slm_act_endpoint) == 0) {
    slm_act_fct();
  }
  if (strcmp(task, slm_set_endpoint) == 0) {
    slm_set_fct();
  }
  if (strcmp(task, slm_get_endpoint) == 0) {
    slm_get_fct();
  }
#endif

  /*
    Drive DAC
  */
#ifdef IS_DAC
  if (strcmp(task, dac_act_endpoint) == 0)
    dac->act();
  if (strcmp(task, dac_set_endpoint) == 0)
    dac->set();
  if (strcmp(task, dac_get_endpoint) == 0)
    dac->get();
#endif

  /*
    Drive Laser
  */
  if (strcmp(task, laser_act_endpoint) == 0)
    laser->act();
  if (strcmp(task, laser_set_endpoint) == 0)
    laser->get();
  if (strcmp(task, laser_get_endpoint) == 0)
    laser->set();

  /*
    Drive analog
  */
#ifdef IS_ANALOG
  if (strcmp(task, analog_act_endpoint) == 0)
    analog->act();
  if (strcmp(task, analog_set_endpoint) == 0)
    analog->set();
  if (strcmp(task, analog_get_endpoint) == 0)
    analog->get();
#endif


  /*
    Drive digital
  */
#ifdef IS_DIGITAL
  if (strcmp(task, digital_act_endpoint) == 0)
   digital->act(&jsonDocument);
  if (strcmp(task, digital_set_endpoint) == 0)
    digital->set(&jsonDocument);
  if (strcmp(task, digital_get_endpoint) == 0)
    digital->get(&jsonDocument);
#endif


  /*
    Drive LED Matrix
  */

  if (strcmp(task, ledarr_act_endpoint) == 0)
    led->act();
  if (strcmp(task, ledarr_set_endpoint) == 0)
    led->set();
  if (strcmp(task, ledarr_get_endpoint) == 0)
    led->get();


  /*
    Read the sensor
  */
#ifdef IS_READSENSOR
  if (strcmp(task, readsensor_act_endpoint) == 0)
    sensor->act();
  if (strcmp(task, readsensor_set_endpoint) == 0)
    sensor->set();
  if (strcmp(task, readsensor_get_endpoint) == 0)
    sensor->get();
#endif

  /*
    Control PID controller
  */
#ifdef IS_PID
  if (strcmp(task, PID_act_endpoint) == 0)
    pid->act();
  if (strcmp(task, PID_set_endpoint) == 0)
    pid->set();
  if (strcmp(task, PID_get_endpoint) == 0)
    pid->get();
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

  motor->isactive = true;
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


  for (int irepeats = 0; irepeats < N_repeats; irepeats++) {
    for (int itask = 0; itask < N_tasks; itask++) {
      char json_string[256];
      // Hacky, but should work
      Serial.println(itask);
      serializeJson(tmpJsonDoc[String(itask)], json_string);
      Serial.println(json_string);
      deserializeJson(jsonDocument, json_string);

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

  motor->isactive = false;
}
