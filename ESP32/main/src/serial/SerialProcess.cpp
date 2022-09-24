#include "SerialProcess.h"

SerialProcess::SerialProcess(/* args */)
{
}

SerialProcess::~SerialProcess()
{
}

void SerialProcess::loop(DynamicJsonDocument * jsonDocument)
{
#ifdef IS_SERIAL
    //Config::loop(); // make it sense to call this everyime?
    if (Serial.available())
    {
        DeserializationError error = deserializeJson((*jsonDocument), Serial);
        // free(Serial);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }
        Serial.flush();

        String task_s = (*jsonDocument)["task"];
        char task[50];
        task_s.toCharArray(task, 256);

// jsonDocument.garbageCollect(); // memory leak?
/*if (task == "null") return;*/
#ifdef DEBUG_MAIN
        Serial.print("TASK: ");
        Serial.println(task);
#endif

        // do the processing based on the incoming stream
        if (strcmp(task, "multitable") == 0)
        {
            // multiple tasks
            tableProcessor(jsonDocument);
        }
        else
        {
            // Process individual tasks
            jsonProcessor(task,jsonDocument);
        }
    }
#endif
}

void SerialProcess::jsonProcessor(String task,DynamicJsonDocument * jsonDocument)
{
  /*
      Return state
  */
  if (task == state_act_endpoint)
    state.act();
  if (task == state_set_endpoint)
    state.set();
  if (task == state_get_endpoint)
    state.get();
/*
  Drive Motors
*/
#ifdef IS_MOTOR
  if (task == motor_act_endpoint)
  {
    motor.act();
  }
  if (task == motor_set_endpoint)
  {
    motor.set();
  }
  if (task == motor_get_endpoint)
  {
    motor.get();
  }
#endif
/*
  Operate SLM
*/
#ifdef IS_SLM
  if (task == slm_act_endpoint)
  {
    slm.act();
  }
  if (task == slm_set_endpoint)
  {
    slm.set();
  }
  if (task == slm_get_endpoint)
  {
    slm.get();
  }
#endif
/*
  Drive DAC
*/
#ifdef IS_DAC
  if (task == dac_act_endpoint)
    dac.act();
  if (task == dac_set_endpoint)
    dac.set();
  if (task == dac_get_endpoint)
    dac.get();
#endif
/*
  Drive Laser
*/
#ifdef IS_LASER
  if (task == laser_act_endpoint)
    laser.act();
  if (task == laser_set_endpoint)
    laser.get();
  if (task == laser_get_endpoint)
    laser.set();
#endif
/*
  Drive analog
*/
#ifdef IS_ANALOG
  if (task == analog_act_endpoint)
    analog.act();
  if (task == analog_set_endpoint)
    analog.set();
  if (task == analog_get_endpoint)
    analog.get();
#endif
/*
  Drive digital
*/
#ifdef IS_DIGITAL
  if (task == digital_act_endpoint)
    digital.act(jsonDocument);
  if (task == digital_set_endpoint)
    digital.set(jsonDocument);
  if (task == digital_get_endpoint)
    digital.get(jsonDocument);
#endif
/*
  Drive LED Matrix
*/
#ifdef IS_LED
  if (task == ledarr_act_endpoint)
    LedController::act();
  if (task == ledarr_set_endpoint)
    LedController::set();
  if (task == ledarr_get_endpoint)
    LedController::get();
#endif
  /*
    Change Configuration
  */
  if (task == config_act_endpoint)
    Config::act();
  if (task == config_set_endpoint)
    Config::set();
  if (task == config_get_endpoint)
    Config::get();

/*
  Read the sensor
*/
#ifdef IS_READSENSOR
  if (task == readsensor_act_endpoint)
    sensor.act();
  if (task == readsensor_set_endpoint)
    sensor.set();
  if (task == readsensor_get_endpoint)
    sensor.get();
#endif

/*
  Control PID controller
*/
#ifdef IS_PID
  if (task == PID_act_endpoint)
    pid.act();
  if (task == PID_set_endpoint)
    pid.set();
  if (task == PID_get_endpoint)
    pid.get();
#endif
  if (task == scanwifi_endpoint)
  {
    RestApi::scanWifi();
  }
  if (task == connectwifi_endpoint)
  {
    RestApi::connectToWifi();
  }
  if (task == reset_nv_flash_endpoint)
  {
    RestApi::resetNvFLash();
  }
  
  
  // Send JSON information back
  Serial.println("++");
  serializeJson((*jsonDocument), Serial);
  Serial.println();
  Serial.println("--");
  jsonDocument->clear();
  jsonDocument->garbageCollect();
}

void SerialProcess::tableProcessor(DynamicJsonDocument * jsonDocument)
{
#ifdef IS_MOTOR
  motor.isactive = true;
#endif
  // 1. Copy the table
  DynamicJsonDocument tmpJsonDoc = (*jsonDocument);
  jsonDocument->clear();

  // 2. now we need to extract the indidvidual tasks
  int N_tasks = tmpJsonDoc["task_n"];
  int N_repeats = tmpJsonDoc["repeats_n"];

  Serial.println("N_tasks");
  Serial.println(N_tasks);
  Serial.println("N_repeats");
  Serial.println(N_repeats);

  for (int irepeats = 0; irepeats < N_repeats; irepeats++)
  {
    for (int itask = 0; itask < N_tasks; itask++)
    {
      char json_string[256];
      // Hacky, but should work
      Serial.println(itask);
      serializeJson(tmpJsonDoc[String(itask)], json_string);
      Serial.println(json_string);
      deserializeJson((*jsonDocument), json_string);

      String task_s = (*jsonDocument)["task"];
      char task[50];
      task_s.toCharArray(task, 256);

// jsonDocument.garbageCollect(); // memory leak?
/*if (task == "null") return;*/
#ifdef DEBUG_MAIN
      Serial.print("TASK: ");
      Serial.println(task);
#endif
      jsonProcessor(task, jsonDocument);
    }
  }
  tmpJsonDoc.clear();
#ifdef IS_MOTOR
  motor.isactive = false;
#endif
}