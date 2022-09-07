#include "DacController.h"




void DacController::setup()
{
  #ifdef IS_DAC
    dac->dac = new DAC_Module();
  #endif
    #ifdef IS_DAC
    dac->dac->Setup(DAC_CHANNEL_1, 1000, 50, 0, 0, 2);
    dac->dac->Setup(DAC_CHANNEL_2, 1000, 50, 0, 0, 2);
    #endif
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
}

// Custom function accessible by the API
void DacController::act() {
  // here you can do something

  Serial.println("dac_act_fct");

  // apply default parameters
  // DAC Channel
  dac_channel = DAC_CHANNEL_1;
  if (jsonDocument->containsKey("dac_channel")) {
    dac_channel = (*jsonDocument)["dac_channel"];
  }

  // DAC Frequency
  frequency = 1000;
  if (jsonDocument->containsKey("frequency")) {
    frequency = (*jsonDocument)["frequency"];
  }

  // DAC offset
  int offset = 0;
  if (jsonDocument->containsKey("offset")) {
    int offset = (*jsonDocument)["offset"];
  }

  // DAC amplitude
  int amplitude = 0;
  if (jsonDocument->containsKey("amplitude")) {
    int amplitude = (*jsonDocument)["amplitude"];
  }

  // DAC clk_div
  int clk_div = 0;
  if (jsonDocument->containsKey("clk_div")) {
    int clk_div = (*jsonDocument)["clk_div"];
  }

  if ((*jsonDocument)["dac_channel"] == 1)
    dac_channel = DAC_CHANNEL_1;
  else if ((*jsonDocument)["dac_channel"] == 2)
    dac_channel = DAC_CHANNEL_2;

  //Scale output of a DAC channel using two bit pattern:
  if (amplitude == 0) scale = 0;
  else if (amplitude == 1) scale = 01;
  else if (amplitude == 2) scale = 10;
  else if (amplitude == 3) scale = 11;


  if (DEBUG) {
    Serial.print("dac_channel "); Serial.println(dac_channel);
    Serial.print("frequency "); Serial.println(frequency);
    Serial.print("offset "); Serial.println(offset);
  }

  #ifdef IS_DAC

  if (dac_is_running)
    if (frequency == 0) {
      dac_is_running = false;
      dac->Stop(dac_channel);
      dacWrite(dac_channel, offset);
    }
    else {
      dac->Stop(dac_channel);
      dac->Setup(dac_channel, clk_div, frequency, scale, phase, invert);
      dac_is_running = true;
    }
  else {
    dac->Setup(dac_channel, clk_div, frequency, scale, phase, invert);
      dac->dac_offset_set(dac_channel, offset);
  }
  #endif

  jsonDocument->clear();
  (*jsonDocument)["return"] = 1;
}

void DacController::set() {
  // here you can set parameters
  int value = (*jsonDocument)["value"];

  if (DEBUG) {
    Serial.print("value "); Serial.println(value);
  }

  int dac_set = (*jsonDocument)["dac_set"];

  if (dac_set != NULL) {
    if (DEBUG) Serial.print("dac_set "); Serial.println(dac_set);
    // SET SOMETHING
  }

  jsonDocument->clear();
  (*jsonDocument)["return"] = 1;

}





// Custom function accessible by the API
void DacController::get() {
  // GET SOME PARAMETERS HERE
  int dac_variable = 12343;

  jsonDocument->clear();
  (*jsonDocument)["dac_variable"] = dac_variable;
}


/*
   wrapper for HTTP requests
*/




static void drive_galvo(void * parameter){
  while(true){ // infinite loop
    digitalWrite(dac->pins->dac_fake_1, HIGH);
    digitalWrite(dac->pins->dac_fake_2, HIGH);
    vTaskDelay(dac->frequency/portTICK_PERIOD_MS); // pause 1ms
    digitalWrite(dac->pins->dac_fake_1, LOW);
    digitalWrite(dac->pins->dac_fake_2, LOW);
    vTaskDelay(dac->frequency/portTICK_PERIOD_MS); // pause 1ms
   }
}
