#include "config.h"

#include "src/motor/FocusMotor.h"
#include "src/led/led_controller.h"
#include "src/laser/LaserController.h"
#include "src/analog/AnalogController.h"
#include "src/state/State.h"
#include "src/wifi/WifiController.h"

#include <ArduinoJson.h>

/*
    IMPORTANT: ALL setup-specific settings can be found in the "pindef.h" files
*/

// For PS4 support, please install this library https://github.com/beniroquai/PS4-esp32/
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"



#ifdef IS_DIGITAL
  #include "parameters_digital.h"
#endif

#ifdef IS_READSENSOR
//#include "parameters_readsensor.h"
#endif

#ifdef IS_PID
#include "parameters_PID.h"
#endif





//Where the JSON for the current instruction lives
#ifdef IS_SLM
  DynamicJsonDocument jsonDocument(32784);
#else
  DynamicJsonDocument jsonDocument(4096);
#endif


// ensure motors switch off when PS3 controller is operating them
bool override_overheating = false;

/*
   Register devices
*/
FocusMotor * focusMotor;
led_controller * led;
LaserController * laser;
PINDEF * pins;
AnalogController * analog;
State * state;
#if defined(IS_DAC) || defined(IS_DAC_FAKE)
  #include "src/dac/DacController.h"
  DacController * dac;
#endif

WifiController * wifi;

#ifdef IS_SLM
#include "parameters_slm.h"
#endif

#ifdef IS_PS3
  #include "src/gamepads/ps3_controller.h"
  ps3_controller gp_controller;
#endif
#ifdef IS_PS4
  #include "gamepads/ps4_controller.h"
  ps4_controller * gp_controller;
#endif

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

  state = new State();
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
  wifi->state = state;
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
if(wifi != nullptr)
  wifi->led = led;
#ifdef DEBUG_LED
  led->DEBUG = true;
#endif
led->jsonDocument = &jsonDocument;
led->setup_matrix();

focusMotor = new FocusMotor(pins);
if(wifi != nullptr)
  wifi->motor = focusMotor;
#ifdef DEBUG_MOTOR
  focusmotor->DEBUG = true;
#endif
focusMotor->jsonDocument = &jsonDocument;
focusMotor->setup_motor();

  state->clearBlueetoothDevice();
  #ifdef IS_PS3
    #ifdef DEBUG_GAMEPAD
      gp_controller.DEBUG = true;
    #endif
    gp_controller.focusmotor = focusMotor;
    gp_controller.led = led;
    gp_controller.start();
  #endif
  #ifdef IS_PS4
    #ifdef DEBUG_GAMEPAD
      gp_controller.DEBUG = true;
    #endif
    gp_controller.focusmotor = &focusMotor;
    gp_controller.led = &led;
    gp_controller->start();
  #endif

  laser = new LaserController();
  laser->setup_laser();

  if(wifi != nullptr)
    wifi->laser = laser;


#if defined IS_DAC || definded IS_DAC_FAKE
  Serial.println("Setting Up DAC");
  dac = new DacController();
  dac->jsonDocument = &jsonDocument;
  dac->pins = pins;
  dac->setup();
  if(wifi != nullptr)
  wifi->dac = dac;
#endif


#ifdef IS_ANALOG
  analog = new AnalogController();
  if(wifi != nullptr)
  wifi->analog = analog;
  #ifdef DEBUG_ANALOG
    analog->DEBUG = true;
  #endif
  analog->jsonDocument = &jsonDocument;
  analog->pins = pins;
  analog->setup();
#endif

#ifdef IS_DIGITAL
  setupDigital();
#endif

  // list modules
if(wifi != nullptr)
    Serial.println("IS_WIFI");
if(led != nullptr)
    Serial.println("IS_LED");
if(analog != nullptr)
    Serial.println("IS_ANALOG");
if(dac != nullptr)
{
  #if defined IS_DAC
    Serial.println("IS_DAC");
  #endif
  #ifdef definded IS_DAC_FAKE
    Serial.println("IS_DAC_FAKE");
  #endif
}
if(focusMotor != nullptr)
  Serial.println("IS_MOTOR");
#ifdef IS_SLM
  Serial.println("IS_SLM");
#endif

#ifdef IS_READSENSOR
  setup_sensors();
  Serial.println(readsensor_act_endpoint);
  Serial.println(readsensor_set_endpoint);
  Serial.println(readsensor_get_endpoint);
#endif

#ifdef IS_PID
  setup_PID();
  Serial.println(PID_act_endpoint);
  Serial.println(PID_set_endpoint);
  Serial.println(PID_get_endpoint);
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


#if defined IS_PS3 || defined IS_PS4
  gp_controller.control(); // if controller is operating motors, overheating protection is enabled
#endif

#ifdef IS_WIFI
  wifi->handelMessages();
#endif

  /*
     continous control during loop
  */
  if (!focusMotor->isstop) {
    focusMotor->isactive = true;
    focusMotor->drive_motor_background();
  }


#ifdef IS_PID
  if (PID_active and (state->currentMillis - state->startMillis >= PID_updaterate)) {
    PID_background();
    startMillis = millis();
  }
#endif


}

void jsonProcessor(char task[]) {


  /*
      Return state
  */
  if (strcmp(task, state_act_endpoint) == 0)
    state->state_act_fct();
  if (strcmp(task, state_set_endpoint) == 0)
    state->state_set_fct();
  if (strcmp(task, state_get_endpoint) == 0)
    state->state_get_fct();

  /*
    Drive Motors
  */
  if (strcmp(task, motor_act_endpoint) == 0) {
    focusMotor->motor_act_fct();
  }
  if (strcmp(task, motor_set_endpoint) == 0) {
    focusMotor->motor_set_fct();
  }
  if (strcmp(task, motor_get_endpoint) == 0) {
    focusMotor->motor_get_fct();
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
    dac->dac_act_fct();
  if (strcmp(task, dac_set_endpoint) == 0)
    dac->dac_set_fct();
  if (strcmp(task, dac_get_endpoint) == 0)
    dac->dac_get_fct();
#endif

  /*
    Drive Laser
  */
  if (strcmp(task, laser_act_endpoint) == 0)
    laser->LASER_act_fct();
  if (strcmp(task, laser_set_endpoint) == 0)
    laser->LASER_get_fct();
  if (strcmp(task, laser_get_endpoint) == 0)
    laser->LASER_set_fct();

  /*
    Drive analog
  */
#ifdef IS_ANALOG
  if (strcmp(task, analog_act_endpoint) == 0)
    analog->analog_act_fct();
  if (strcmp(task, analog_set_endpoint) == 0)
    analog->analog_set_fct();
  if (strcmp(task, analog_get_endpoint) == 0)
    analog->analog_get_fct();
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

  if (strcmp(task, ledarr_act_endpoint) == 0)
    led->ledarr_act_fct();
  if (strcmp(task, ledarr_set_endpoint) == 0)
    led->ledarr_set_fct();
  if (strcmp(task, ledarr_get_endpoint) == 0)
    led->ledarr_get_fct();


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

  /*
    Control PID controller
  */
#ifdef IS_PID
  if (strcmp(task, PID_act_endpoint) == 0)
    PID_act_fct();
  if (strcmp(task, PID_set_endpoint) == 0)
    PID_set_fct();
  if (strcmp(task, PID_get_endpoint) == 0)
    PID_get_fct();
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

  focusMotor->isactive = true;
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

  focusMotor->isactive = false;
}
