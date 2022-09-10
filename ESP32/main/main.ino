#include "config.h"
#include <ArduinoJson.h>
#ifdef IS_MOTOR
#include "src/motor/FocusMotor.h"
#endif
#ifdef IS_LED
  #include "src/led/led_controller.h"
#endif
#ifdef IS_LASER
  #include "src/laser/LaserController.h"
#include "parameters_ledarr.h"
#include "parameters_config.h"

// We use the strip instead of the matrix to ensure different dimensions; Convesion of the pattern has to be done on the cliet side!
Adafruit_NeoPixel* matrix = NULL;





// define permanent flash object
Preferences preferences;
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
#if defined IS_DAC || defined IS_DAC_FAKE
  #include "src/dac/DacController.h"
#endif
#ifdef IS_SLM
  #include "src/slm/SlmController.h"
#endif
#if defined IS_PS4 || defined IS_PS3
  #include "src/gamepads/ps_3_4_controller.h"
#endif
#include "src/wifi/WifiController.h"
#include "src/config/ConfigController.h"

#include <ArduinoJson.h>
/*
    IMPORTANT: ALL setup-specific settings can be found in the "pindef.h" files
*/
// For PS4 support, please install this library https://github.com/beniroquai/PS4-esp32/
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
  DynamicJsonDocument jsonDocument(32784);


PINDEF * pins;
ConfigController configController;
/*State state;
#if defined IS_PS3 || defined IS_PS4
    ps_3_4_controller ps_c;
#endif
#if defined IS_DAC || defined IS_DAC_FAKE
    DacController dac;
    AnalogController analog;
#ifdef IS_LASER
    LaserController laser;
#ifdef IS_LED
    led_controller led;
#ifdef IS_MOTOR
    FocusMotor motor;
    PidController pid;
#endif
#ifdef IS_SCANNER
    ScannerController scanner;
#endif
#ifdef IS_READSENSOR
    SensorController sensor;
#endif
#ifdef IS_DIGITAL
    DigitalController digital;
#endif
#ifdef IS_SLM
    SlmController slm;
#ifdef IS_WIFI
    WifiController wifi;
#endif*/
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
  state.startMillis = millis();
  pins = new PINDEF();
  state.getDefaultPinDef((*pins));
  // Start Serial
  Serial.begin(BAUDRATE);
  Serial.println("Start");
  state.printInfo();

  // if we boot for the first time => reset the preferences! // TODO: Smart? If not, we may have the problem that a wrong pin will block bootup
  if (isFirstRun()) {
    Serial.println("First Run, resetting config?");
    resetConfigurations();
  }

  // check if setup went through after new config - avoid endless boot-loop
  preferences.begin("setup", false);
  if (preferences.getBool("setupComplete", true) == false) {
    Serial.println("Setup not done, resetting config?"); //TODO not working!
    resetConfigurations();
  }
  else{
    Serial.println("Setup done, continue.");
  }
  preferences.putBool("setupComplete", false);
  preferences.end();





  
  // reset jsonDocument
  jsonDocument.clear();
    // load config
  configController.setup();

  // connect to wifi if necessary
  bool isResetWifiSettings = false;
  wifi.autoconnectWifi(isResetWifiSettings);
  wifi.setup_routing();
  wifi.init_Spiffs();
  startServer();

  Serial.println(state_act_endpoint);
  Serial.println(state_get_endpoint);
  Serial.println(state_set_endpoint);

#ifdef IS_SLM
  Serial.println("IS_SLM");
  slm.jsonDocument = &jsonDocument;
  slm.setup();
#endif
#ifdef IS_LED
  Serial.println("IS_LED");
  #ifdef DEBUG_LED
    led.DEBUG = true;
  Serial.println(PS4_MACADDESS);
  PS4.begin("1a:2b:4c:01:01:01");
#endif
  led.jsonDocument = &jsonDocument;
  led.setup_matrix();
#endif

#ifdef IS_MOTOR
  Serial.println("IS_MOTOR");
  #ifdef DEBUG_MOTOR
    motor->DEBUG = true;
  #endif
  motor.jsonDocument = &jsonDocument;
  motor.pins = pins;
  motor.setup();
#endif

#if defined IS_PS4 || defined IS_PS3
  state.clearBlueetoothDevice();
  #ifdef DEBUG_GAMEPAD
    ps_c.DEBUG = true;
  #endif
  ps_c.start();
#endif

#ifdef IS_LASER
  laser.jsonDocument = &jsonDocument;
  laser.pins = pins;
  laser.setup();
#endif

#if defined IS_DAC || defined IS_DAC_FAKE
  #if defined IS_DAC
    Serial.println("IS_DAC");
  #endif
  #if defined IS_DAC_FAKE
    Serial.println("IS_DAC_FAKE");
  #endif
  dac.jsonDocument = &jsonDocument;
  dac.pins = pins;
  //wifi->dac = &dac;
  dac.setup();
#endif

#ifdef IS_ANALOG
  Serial.println("IS_ANALOG");
  #ifdef DEBUG_ANALOG
    analog->DEBUG = true;
  #endif
  analog.jsonDocument = &jsonDocument;
  analog.pins = pins;
  analog.setup();
#endif

#ifdef IS_DIGITAL
  digital.pins = pins;
  digital.setup();
#endif

#ifdef IS_READSENSOR
  Serial.println("IS_SENSOR");
  sensor.jsonDocument = &jsonDocument;
  sensor.pins = pins;
  //wifi.sensor = &sensor;
  sensor.setup();
  Serial.println(readsensor_act_endpoint);
  Serial.println(readsensor_set_endpoint);
  Serial.println(readsensor_get_endpoint);
#endif

#ifdef IS_PID
  Serial.println("IS_PID");
  pid.jsonDocument = &jsonDocument;
  pid.pins = pins;
  pid.setup();
  Serial.println(PID_act_endpoint);
  Serial.println(PID_set_endpoint);
  Serial.println(PID_get_endpoint);
#endif
#ifdef IS_SCANNER
  Serial.println("IS_SCANNER");
  scanner.pins = pins;
  //wifi.scanner = &scanner;
  scanner.setup();
#endif
}

//char *task = strdup("");
//char* task = "";

void loop() {
  // handle any http requests
  server.handleClient();

  // for any timing-related purposes
  state.currentMillis = millis();

#ifdef IS_SERIAL
  configController.loop(); //make it sense to call this everyime?
  if (Serial.available()) {
    DeserializationError error = deserializeJson(jsonDocument, Serial);
    //free(Serial);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    Serial.flush();
    #ifdef DEBUG_MAIN
      serializeJsonPretty(jsonDocument, Serial);
    #endif


    String task_s = jsonDocument["task"];
    char task[50];
    task_s.toCharArray(task, 256);


    //jsonDocument.garbageCollect(); // memory leak?
    /*if (task == "null") return;*/
    #ifdef DEBUG_MAIN
      Serial.print("TASK: ");
      Serial.println(task);
    #endif

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
#ifdef IS_LASER
  // attempting to despeckle by wiggeling the temperature-dependent modes of the laser?
  if (laser.LASER_despeckle_1 > 0 && laser.LASER_val_1 > 0)
    laser.LASER_despeckle(laser.LASER_despeckle_1, 1, laser.LASER_despeckle_period_1);
  if (laser.LASER_despeckle_2 > 0 && laser.LASER_val_2 > 0)
    laser.LASER_despeckle(laser.LASER_despeckle_2, 2, laser.LASER_despeckle_period_2);
  if (laser.LASER_despeckle_3 > 0 && laser.LASER_val_3 > 0)
    laser.LASER_despeckle(laser.LASER_despeckle_3, 3, laser.LASER_despeckle_period_3);
#endif

#if defined IS_PS4 || defined IS_PS3
  ps_c.control(); // if controller is operating motors, overheating protection is enabled
#endif
#ifdef IS_WIFI
  wifi.handelMessages();
#endif

/*
    continous control during loop
*/
  if (!motor.isstop) {
    motor.isactive = true;
    motor.background();
  }
#ifdef IS_PID
  if (pid.PID_active && (state.currentMillis - state.startMillis >= pid.PID_updaterate)) {
    pid.background();
    state.startMillis = millis();
  }
#endif
}

void jsonProcessor(char task[]) {
/*
    Return state
*/
  if (strcmp(task, state_act_endpoint) == 0)
    state.act();
  if (strcmp(task, state_set_endpoint) == 0)
    state.set();
  if (strcmp(task, state_get_endpoint) == 0)
    state.get();
/*
  Drive Motors
*/
#ifdef IS_MOTOR
  if (strcmp(task, motor_act_endpoint) == 0) {
    motor.act();
  }
  if (strcmp(task, motor_set_endpoint) == 0) {
    motor.set();
  }
  if (strcmp(task, motor_get_endpoint) == 0) {
    motor.get();
  }
#endif
/*
  Operate SLM
*/
#ifdef IS_SLM
  if (strcmp(task, slm_act_endpoint) == 0) {
    slm.act();
  }
  if (strcmp(task, slm_set_endpoint) == 0) {
    slm.set();
  }
  if (strcmp(task, slm_get_endpoint) == 0) {
    slm.get();
  }
#endif
/*
  Drive DAC
*/
#ifdef IS_DAC
  if (strcmp(task, dac_act_endpoint) == 0)
    dac.act();
  if (strcmp(task, dac_set_endpoint) == 0)
    dac.set();
  if (strcmp(task, dac_get_endpoint) == 0)
    dac.get();
#endif
/*
  Drive Laser
*/
#ifdef IS_LASER
  if (strcmp(task, laser_act_endpoint) == 0)
    laser.act();
  if (strcmp(task, laser_set_endpoint) == 0)
    laser.get();
  if (strcmp(task, laser_get_endpoint) == 0)
    laser.set();
#endif
/*
  Drive analog
*/
#ifdef IS_ANALOG
  if (strcmp(task, analog_act_endpoint) == 0)
    analog.act();
  if (strcmp(task, analog_set_endpoint) == 0)
    analog.set();
  if (strcmp(task, analog_get_endpoint) == 0)
    analog.get();
#endif
/*
  Drive digital
*/
#ifdef IS_DIGITAL
  if (strcmp(task, digital_act_endpoint) == 0)
   digital.act(&jsonDocument);
  if (strcmp(task, digital_set_endpoint) == 0)
    digital.set(&jsonDocument);
  if (strcmp(task, digital_get_endpoint) == 0)
    digital.get(&jsonDocument);
#endif
/*
  Drive LED Matrix
*/
#ifdef IS_LED
  if (strcmp(task, ledarr_act_endpoint) == 0)
    led.act();
  if (strcmp(task, ledarr_set_endpoint) == 0)
    led.set();
  if (strcmp(task, ledarr_get_endpoint) == 0)
    led.get();
#endif
  /*
    Change Configuration
  */
  if (strcmp(task, config_act_endpoint) == 0)
    config.act();
  if (strcmp(task, config_set_endpoint) == 0)
    config.set();
  if (strcmp(task, config_get_endpoint) == 0)
    config.get();

/*
  Read the sensor
*/
#ifdef IS_READSENSOR
  if (strcmp(task, readsensor_act_endpoint) == 0)
    sensor.act();
  if (strcmp(task, readsensor_set_endpoint) == 0)
    sensor.set();
  if (strcmp(task, readsensor_get_endpoint) == 0)
    sensor.get();
#endif

/*
  Control PID controller
*/
#ifdef IS_PID
  if (strcmp(task, PID_act_endpoint) == 0)
    pid.act();
  if (strcmp(task, PID_set_endpoint) == 0)
    pid.set();
  if (strcmp(task, PID_get_endpoint) == 0)
    pid.get();
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
#ifdef IS_MOTOR
  motor.isactive = true;
#endif
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
      #ifdef DEBUG_MAIN
        Serial.print("TASK: ");
        Serial.println(task);
      #endif
      jsonProcessor(task);
    }
  }
  tmpJsonDoc.clear();
#ifdef IS_MOTOR
  motor.isactive = false;
#endif
}
