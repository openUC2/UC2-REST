//#define IS_PS3
#define IS_PS4

// external headers
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#ifdef IS_PS3
#include <Ps3Controller.h>
#else
#include <PS4Controller.h>
#endifb
#include <Arduino.h>
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include"esp_gap_bt_api.h"
#include "esp_err.h"
#include "Preferences.h"
#include <Preferences.h>
#include <ArduinoJson.h>


// internal headers
#include "parameters_state.h"
#include "parameters_laser.h"
#include "parameters_motor.h"
#include "parameters_ledarr.h"
#include "pindef.h" // for pin definitions
//#include "pindefUC2Standalone.h"
#include "parameters_ps.h" // playstation parameters
#include "parameters_config.h"

// We use the strip instead of the matrix to ensure different dimensions; Convesion of the pattern has to be done on the cliet side!
Adafruit_NeoPixel* matrix = NULL;





// define permanent flash object
Preferences preferences;

#if defined(IS_DAC) || defined(IS_DAC_FAKE)
#include "parameters_dac.h"
#endif

int DEBUG = 1; // if tihs is set to true, the arduino runs into problems during multiple serial prints..
#define BAUDRATE 115200

#include "parameters_wifi.h"
WiFiManager wm;

#ifdef IS_ANALOG
#include "parameters_analog.h"
#endif
#ifdef IS_DIGITAL
#include "parameters_digital.h"
#endif

#ifdef IS_READSENSOR
//#include "parameters_readsensor.h"
#endif

#ifdef IS_PID
#include "parameters_PID.h"
#endif

DynamicJsonDocument jsonDocument(32784);

WebServer server(80);
char output[1000];

#ifdef IS_DAC
DAC_Module *dac = new DAC_Module();
#endif

/*
   Register devices
*/
AccelStepper stepper_A;
AccelStepper stepper_X;
AccelStepper stepper_Y;
AccelStepper stepper_Z;

#ifdef IS_SLM
#include "parameters_slm.h"
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

  // for any timing related puposes..
  startMillis = millis();

  // Start Serial
  Serial.begin(BAUDRATE);
  Serial.println("Start");


  // if we boot for the first time => reset the preferences! // TODO: Smart? If not, we may have the problem that a wrong pin will block bootup
  if (isFirstRun()) {
    Serial.println("First Run, resetting config?");
    resetConfigurations();
  }

  // check if setup went through after new config - avoid endless boot-loop
  preferences.begin("setup", false);
  if (preferences.getBool("setupComplete", true) == false) {
    
    // make sure next time no bootloop is created
    preferences.putBool("setupComplete", false);
    preferences.end();
    
    Serial.println("Setup not done, resetting config?"); //TODO not working!
    resetConfigurations();

  }
  else {
    Serial.println("Setup done, continue.");
  }
  preferences.putBool("setupComplete", false);
  preferences.end();

  // load config
  loadConfiguration();

  // display state
  printInfo();

  // reset jsonDocument
  jsonDocument.clear();

  // connect to wifi if necessary
  bool isResetWifiSettings = false;
  //autoconnectWifi(isResetWifiSettings);
  initWifiAP(WifiSSIDAP);
  setupRouting();
  startServer();
  init_Spiffs();

  Serial.println(state_act_endpoint);
  Serial.println(state_get_endpoint);
  Serial.println(state_set_endpoint);

#ifdef IS_SLM
  setup_slm();
#endif

  setup_matrix();
  setup_motor();
  setup_laser();

  /*
    setting up playstation controller
  */
  if(isFirstRun()) {
    // 
    clearBlueetoothDevice();
    ESP.restart();
  }

#ifdef IS_PS3
  Serial.println("Connnecting to the PS3 controller, please please the magic round button in the center..");
  Ps3.attach(onAttachPS3);
  Ps3.attachOnConnect(onConnectPS3);
  Ps3.attachOnDisconnect(onDisConnectPS3);
  const char* PS3_MACADDESS = "1a:2b:3c:01:01:01";
  Serial.println("PS3 controler is set up.");
  Ps3.begin("1a:2b:3c:01:01:01");
  Serial.println(PS3_MACADDESS);
#else
  //String address = Ps3.getAddress(); // have arbitrary address?
  //Serial.println(address);
  Serial.println("PS4 controler is set up.");
  Serial.println("Connnecting to the PS4 controller, please please the magic round button in the center..");
  const char*  PS4_MACADDESS = "1a:2b:3c:01:01:01"; // TODO not working with adaptive address
  Serial.println(PS4Mac);
  Serial.println(PS4_MACADDESS);
  PS4.begin(PS4_MACADDESS);
  PS4.attach(onAttachPS4);
  PS4.attachOnConnect(onConnectPS4);
  PS4.attachOnDisconnect(onDisConnectPS4);
  Serial.print("PS4 controler is set up. - UNICAST!");
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
  ledcAttachPin(ANALOG_PIN_1, PWM_CHANNEL_analog_1);
  ledcWrite(PWM_CHANNEL_analog_1, 0);

  ledcSetup(PWM_CHANNEL_analog_2, pwm_frequency, pwm_resolution);
  ledcAttachPin(ANALOG_PIN_2, PWM_CHANNEL_analog_2);
  ledcWrite(PWM_CHANNEL_analog_2, 0);
#endif

#ifdef IS_DIGITAL
  setupDigital();
#endif

  // list modules
#ifdef IS_SLM
  Serial.println("IS_SLM");
#endif

#ifdef IS_DAC
  Serial.println(dac_act_endpoint);
  Serial.println(dac_get_endpoint);
  Serial.println(dac_set_endpoint);
#endif
  Serial.println(motor_act_endpoint);
  Serial.println(motor_get_endpoint);
  Serial.println(motor_set_endpoint);

  Serial.println(laser_act_endpoint);
  Serial.println(laser_get_endpoint);
  Serial.println(laser_set_endpoint);
#ifdef IS_ANALOG
  Serial.println(analog_act_endpoint);
  Serial.println(analog_get_endpoint);
  Serial.println(analog_set_endpoint);
#endif
#ifdef IS_SLM
  Serial.println(slm_act_endpoint);
  Serial.println(slm_get_endpoint);
  Serial.println(slm_set_endpoint);
#endif
#ifdef IS_DIGITALGOUT
  Serial.println(digital_act_endpoint);
  Serial.println(digital_get_endpoint);
  Serial.println(digital_set_endpoint);
#endif

  Serial.println(ledarr_act_endpoint);
  Serial.println(ledarr_get_endpoint);
  Serial.println(ledarr_set_endpoint);

  Serial.println(config_act_endpoint);
  Serial.println(config_get_endpoint);
  Serial.println(config_set_endpoint);

#ifdef IS_DAC_FAKE
  pinMode(DAC_FAKE_PIN_1, OUTPUT);
  pinMode(DAC_FAKE_PIN_2, OUTPUT);
  frequency = 1;
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

#ifdef IS_PID
  setup_PID();
  Serial.println(PID_act_endpoint);
  Serial.println(PID_set_endpoint);
  Serial.println(PID_get_endpoint);
#endif


  // check if setup went through after new config
  preferences.begin("setup", false);
  preferences.putBool("setupComplete", true);
  preferences.end();
}

//char *task = strdup("");
//char* task = "";

void loop() {
  // handle any http requests
  server.handleClient();

  // for any timing-related purposes
  currentMillis = millis();

  // process incoming serial commands
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

  /*
     continous control during loop
  */
  // attempting to despeckle by wiggeling the temperature-dependent modes of the laser?
  if (LASER_despeckle_1 > 0 and LASER_val_1 > 0)
    LASER_despeckle(LASER_despeckle_1, 1, LASER_despeckle_period_1);
  if (LASER_despeckle_2 > 0 and LASER_val_2 > 0)
    LASER_despeckle(LASER_despeckle_2, 2, LASER_despeckle_period_2);
  if (LASER_despeckle_3 > 0 and LASER_val_3 > 0)
    LASER_despeckle(LASER_despeckle_3, 3, LASER_despeckle_period_3);


#ifdef IS_PS3
  control_PS3(); // if controller is operating motors, overheating protection is enabled
#else
  control_PS4();
#endif

  /*
     continous control during loop
  */
  if (not isstop) {
    isactive = true;
    drive_motor_background();
  }


#ifdef IS_PID
  if (PID_active and (currentMillis - startMillis >= PID_updaterate)) {
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
    state_act_fct();
  if (strcmp(task, state_set_endpoint) == 0)
    state_set_fct();
  if (strcmp(task, state_get_endpoint) == 0)
    state_get_fct();

  /*
    Drive Motors
  */
  if (strcmp(task, motor_act_endpoint) == 0) {
    motor_act_fct();
  }
  if (strcmp(task, motor_set_endpoint) == 0) {
    motor_set_fct();
  }
  if (strcmp(task, motor_get_endpoint) == 0) {
    motor_get_fct();
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
    dac_act_fct();
  if (strcmp(task, dac_set_endpoint) == 0)
    dac_set_fct();
  if (strcmp(task, dac_get_endpoint) == 0)
    dac_get_fct();
#endif

  /*
    Drive Laser
  */
  if (strcmp(task, laser_act_endpoint) == 0)
    LASER_act_fct();
  if (strcmp(task, laser_set_endpoint) == 0)
    LASER_get_fct();
  if (strcmp(task, laser_get_endpoint) == 0)
    LASER_set_fct();

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

  if (strcmp(task, ledarr_act_endpoint) == 0)
    ledarr_act_fct();
  if (strcmp(task, ledarr_set_endpoint) == 0)
    ledarr_set_fct();
  if (strcmp(task, ledarr_get_endpoint) == 0)
    ledarr_get_fct();

  /*
    Change Configuration
  */
  if (strcmp(task, config_act_endpoint) == 0)
    config_act_fct();
  if (strcmp(task, config_set_endpoint) == 0)
    config_set_fct();
  if (strcmp(task, config_get_endpoint) == 0)
    config_get_fct();

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

  if (DEBUG) Serial.println("JSON processed");
}


void tableProcessor() {

  isactive = true;
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

  isactive = false;
}
