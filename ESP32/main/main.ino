#include "config.h"

#include "src/motor/FocusMotor.h"
#include "src/led/led_controller.h"
#include "src/laser/LaserController.h"
/*
    IMPORTANT: ALL setup-specific settings can be found in the "pindef.h" files
*/

// For PS4 support, please install this library https://github.com/beniroquai/PS4-esp32/
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#ifdef IS_WIFI
  #include "parameters_wifi.h"
  WiFiManager wm;
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

#ifdef IS_PID
#include "parameters_PID.h"
#endif


#include <ArduinoJson.h>
#include "parameters_state.h"
#if defined(IS_DAC) || defined(IS_DAC_FAKE)
#include "parameters_dac.h"
uint32_t frequency = 1000;
#endif


//Where the JSON for the current instruction lives
#ifdef IS_SLM
  DynamicJsonDocument jsonDocument(32784);
#else
  DynamicJsonDocument jsonDocument(4096);
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

/*
   Register devices
*/
FocusMotor * focusMotor;
led_controller * led;
LaserController * laser;
PINDEF * pins;



#ifdef IS_SLM
#include "parameters_slm.h"
#endif

#include "parameters_laser.h"

/*
   Register functions
*/

const char* state_act_endpoint = "/state_act";
const char* state_set_endpoint = "/state_set";
const char* state_get_endpoint = "/state_get";

const char* laser_act_endpoint = "/laser_act";
const char* laser_set_endpoint = "/laser_set";
const char* laser_get_endpoint = "/laser_get";

const char* motor_act_endpoint = "/motor_act";
const char* motor_set_endpoint = "/motor_set";
const char* motor_get_endpoint = "/motor_get";

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

const char* ledarr_act_endpoint = "/ledarr_act";
const char* ledarr_set_endpoint = "/ledarr_set";
const char* ledarr_get_endpoint = "/ledarr_get";

#ifdef IS_SLM
const char* slm_act_endpoint = "/slm_act";
const char* slm_set_endpoint = "/slm_set";
const char* slm_get_endpoint = "/slm_get";
#endif

#ifdef IS_READSENSOR
const char* readsensor_act_endpoint = "/readsensor_act";
const char* readsensor_set_endpoint = "/readsensor_set";
const char* readsensor_get_endpoint = "/readsensor_get";
#endif

#ifdef IS_PID
const char* PID_act_endpoint = "/PID_act";
const char* PID_set_endpoint = "/PID_set";
const char* PID_get_endpoint = "/PID_get";
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


  // for any timing related puposes..
  startMillis = millis();
  pins = new PINDEF();
  getDefaultPinDef((*pins));
  // Start Serial
  Serial.begin(BAUDRATE);
  Serial.println("Start");
  printInfo();

  jsonDocument.clear();

  // connect to wifi if necessary
#ifdef IS_WIFI
  bool isResetWifiSettings = false;
  autoconnectWifi(isResetWifiSettings);
  setup_routing();
  init_Spiffs();
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

focusMotor = new FocusMotor(pins);
#ifdef DEBUG_MOTOR
  focusmotor->DEBUG = true;
#endif
focusMotor->jsonDocument = &jsonDocument;
focusMotor->setup_motor();

  clearBlueetoothDevice();
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
#ifdef IS_WIFI
  Serial.println("IS_WIFI");
#endif
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



#ifdef IS_DAC_FAKE
  pinMode(dac_fake_1, OUTPUT);
  pinMode(dac_fake_2, OUTPUT);
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
}

//char *task = strdup("");
//char* task = "";

void loop() {
  // for any timing-related purposes
  currentMillis = millis();

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
  if (LASER_despeckle_1 > 0 and LASER_val_1 > 0)
    laser->LASER_despeckle(LASER_despeckle_1, 1, LASER_despeckle_period_1);
  if (LASER_despeckle_2 > 0 and LASER_val_2 > 0)
    laser->LASER_despeckle(LASER_despeckle_2, 2, LASER_despeckle_period_2);
  if (LASER_despeckle_3 > 0 and LASER_val_3 > 0)
    laser->LASER_despeckle(LASER_despeckle_3, 3, LASER_despeckle_period_3);


#if defined IS_PS3 || defined IS_PS4
  gp_controller.control(); // if controller is operating motors, overheating protection is enabled
#endif

#ifdef IS_WIFI
  server.handleClient();
#endif

  /*
     continous control during loop
  */
  if (!focusMotor->isstop) {
    focusMotor->isactive = true;
    focusMotor->drive_motor_background();
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
