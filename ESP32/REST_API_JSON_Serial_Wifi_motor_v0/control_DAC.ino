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
DynamicJsonDocument DAC_act_fct(JsonDocument& Values) {
  // here you can do something

  Serial.println("DAC_act_fct");

  if (Values["dac_channel"] == 1)
    dac_channel = DAC_CHANNEL_1;
  else if (Values["dac_channel"] == 2)
    dac_channel = DAC_CHANNEL_2;
  frequency = Values["frequency"];
  int offset = Values["offset"];
  int amplitude = Values["amplitude"];

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

  Values.clear();
  Values["return"] = 1;



  return Values ;
}

DynamicJsonDocument DAC_set_fct(JsonDocument& Values) {
  // here you can set parameters
  int value = Values["value"];

  if (DEBUG) {
    Serial.print("value "); Serial.println(value);
  }

  int DAC_set = jsonDocument["DAC_set"];

  if (DAC_set != NULL) {
    if (DEBUG) Serial.print("DAC_set "); Serial.println(DAC_set);
    // SET SOMETHING
  }

  Values.clear();
  Values["return"] = 1;

  return Values ;
}





// Custom function accessible by the API
DynamicJsonDocument DAC_get_fct(JsonDocument& Values) {
  // GET SOME PARAMETERS HERE
  int DAC_variable = 12343;

  jsonDocument.clear();
  jsonDocument["DAC_variable"] = DAC_variable;
  return jsonDocument;
}


/*
   wrapper for HTTP requests
*/


#ifdef IS_WIFI 
void DAC_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = DAC_act_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void DAC_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = DAC_get_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}


// wrapper for HTTP requests
void DAC_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  jsonDocument = DAC_set_fct(jsonDocument);
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
