#ifdef IS_DAC


// DAC-specific parameters
dac_channel_t dac_channel = DAC_CHANNEL_1;
uint32_t clk_div = 0;
uint32_t scale = 0;
uint32_t invert = 2;
uint32_t phase = 0;
uint32_t frequency = 1000;

boolean dac_is_running = false;


// Custom function accessible by the API
void dac_act_fct() {
  // here you can do something

  Serial.println("dac_act_fct");

  if (jsonDocument["dac_channel"] == 1)
    dac_channel = DAC_CHANNEL_1;
  else if (jsonDocument["dac_channel"] == 2)
    dac_channel = DAC_CHANNEL_2;
  frequency = jsonDocument["frequency"];
  int offset = jsonDocument["offset"];
  int amplitude = jsonDocument["amplitude"];

  //Scale output of a DAC channel using two bit pattern:
  if (amplitude == 0 or amplitude == NULL) scale = 0;
  else if (amplitude == 1) scale = 01;
  else if (amplitude == 2) scale = 10;
  else if (amplitude == 3) scale = 11;


  if (DEBUG) {
    Serial.print("dac_channel "); Serial.println(dac_channel);
    Serial.print("frequency "); Serial.println(frequency);
    Serial.print("offset "); Serial.println(offset);
  }

  if (dac_is_running)
    if(frequency==0){
      dac_is_running=false;
      dac->Stop(dac_channel);
      dacWrite(dac_channel, offset);
    }
    else{
      dac->Stop(dac_channel);
      dac->Setup(dac_channel, clk_div, frequency, scale, phase, invert);
      dac_is_running = true;
    }
  else {
    dac->Setup(dac_channel, clk_div, frequency, scale, phase, invert);
    if (offset != NULL)
      dac->dac_offset_set(dac_channel, offset);
  }

  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void dac_set_fct() {
  // here you can set parameters
  int value = jsonDocument["value"];

  if (DEBUG) {
    Serial.print("value "); Serial.println(value);
  }

  int dac_set = jsonDocument["dac_set"];

  if (dac_set != NULL) {
    if (DEBUG) Serial.print("dac_set "); Serial.println(dac_set);
    // SET SOMETHING
  }

  jsonDocument.clear();
  jsonDocument["return"] = 1;

}





// Custom function accessible by the API
void dac_get_fct() {
  // GET SOME PARAMETERS HERE
  int dac_variable = 12343;

  jsonDocument.clear();
  jsonDocument["dac_variable"] = dac_variable;
}


/*
   wrapper for HTTP requests
*/


#ifdef IS_WIFI 
void dac_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  dac_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void dac_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  dac_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}


// wrapper for HTTP requests
void dac_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  dac_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
