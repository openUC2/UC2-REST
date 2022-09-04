


// Custom function accessible by the API
void ledarr_act_fct() {

  // here you can do something
  if (DEBUG) Serial.println("ledarr_act_fct");
  isBusy = true;


  const char* LEDArrMode = jsonDocument["LEDArrMode"]; // "array", "full", "full", "single", "off", "left", "right", "top", "bottom",

  // individual pattern gets adressed
  // PYTHON: send_LEDMatrix_array(self, led_pattern, timeout=1)
  if (strcmp(LEDArrMode, "array") == 0) {
    if (DEBUG) Serial.println("pattern");
    for (int iled = 0; iled < LED_COUNT; iled++) { //Iterate through results
      int red = jsonDocument["red"][iled];  //Implicit cast
      int green = jsonDocument["green"][iled];  //Implicit cast
      int blue = jsonDocument["blue"][iled];  //Implicit cast
      set_led_RGB(iled, red, green, blue);
    }
  }
  // only if a single led will be updated, all others stay the same
  // PYTHON: send_LEDMatrix_single(self, indexled=0, intensity=(255,255,255), timeout=1)
  else if (strcmp(LEDArrMode, "single") == 0) {
    if (DEBUG) Serial.println("single");
    int indexled = jsonDocument["indexled"];
    int red = jsonDocument["red"];  //Implicit cast
    int green = jsonDocument["green"];  //Implicit cast
    int blue = jsonDocument["blue"];  //Implicit cast
    //if (DEBUG) Serial.print(red); Serial.print(green); Serial.println(blue);
    set_led_RGB(indexled, red, green, blue);
  }
  // only few leds will be updated, all others stay the same
  // PYTHON: send_LEDMatrix_multi(self, indexled=(0), intensity=((255,255,255)), Nleds=8*8, timeout=1)
  else if (strcmp(LEDArrMode, "multi") == 0) {
    if (DEBUG) Serial.println("multi");
    int Nleds = jsonDocument["Nleds"];
    for (int i = 0; i < Nleds; i++) { //Iterate through results
      int indexled = jsonDocument["indexled"][i];
      int red = jsonDocument["red"][indexled];  //Implicit cast
      int green = jsonDocument["green"][indexled];  //Implicit cast
      int blue = jsonDocument["blue"][indexled];  //Implicit cast
      //if (DEBUG) Serial.print(red); Serial.print(green); Serial.println(blue);
      set_led_RGB(indexled, red, green, blue);    }
  }
  // turn on all LEDs
  // PYTHON: send_LEDMatrix_full(self, intensity = (255,255,255),timeout=1)
  else if (strcmp(LEDArrMode, "full") == 0) {
    if (DEBUG) Serial.println("full");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    set_all(red, green, blue);
  }
  // turn off all LEDs
  else if (strcmp(LEDArrMode, "left") == 0) {
    if (DEBUG) Serial.println("left");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    int NLeds = jsonDocument["NLeds"];
    set_left(NLeds, red, green, blue);
  }
  // turn off all LEDs
  else if (strcmp(LEDArrMode, "right") == 0) {
    if (DEBUG) Serial.println("right");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    int NLeds = jsonDocument["NLeds"];
    set_right(NLeds, red, green, blue);
  }
  // turn off all LEDs
  else if (strcmp(LEDArrMode, "top") == 0) {
    if (DEBUG) Serial.println("top");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    int NLeds = jsonDocument["NLeds"];
    set_top(NLeds, red, green, blue);
  }
  // turn off all LEDs
  else if (strcmp(LEDArrMode, "bottom") == 0) {
    if (DEBUG) Serial.println("bottom");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    int NLeds = jsonDocument["NLeds"];
    set_bottom(NLeds, red, green, blue);
  }
  jsonDocument.clear();
  jsonDocument["return"] = 1;
  jsonDocument["LEDArrMode"] = LEDArrMode;
  isBusy = false;

}

void ledarr_set_fct() {

  Serial.println("Updating Hardware config of LED Array");

  jsonDocument.clear();
  jsonDocument["return"] = 1;
}



// Custom function accessible by the API
void ledarr_get_fct() {
  jsonDocument.clear();
  jsonDocument["LED_ARRAY_PIN"] = LED_ARRAY_PIN;

}



/***************************************************************************************************/
/*******************************************  LED Array  *******************************************/
/***************************************************************************************************/
/*******************************FROM OCTOPI ********************************************************/

void set_led_RGB(int iLed, int R, int G, int B)  {
  matrix.setPixelColor(iLed, matrix.Color(R,   G,   B));         //  Set pixel's color (in RAM)
  matrix.show();                          //  Update strip to match
}

void setup_matrix() {
  // LED Matrix
  if(DEBUG) Serial.println("Setting up LED array");
  if(DEBUG) Serial.println("LED_ARRAY_PIN: " + String(LED_ARRAY_PIN));
  if(DEBUG) Serial.println("LED_COUNT: " + String(LED_COUNT));
  matrix = Adafruit_NeoPixel(LED_COUNT, LED_ARRAY_PIN, NEO_GRB + NEO_KHZ800);
  matrix.begin();
  matrix.setBrightness(255);
  for(int iLed=0; iLed<LED_COUNT;iLed++){
  matrix.setPixelColor(iLed, matrix.Color(100,100,100));         //  Set pixel's color (in RAM)
  matrix.show();
  }
}

void set_all(int R, int G, int B)
{
  for (int i = 0; i < (LED_COUNT); i++) {
    set_led_RGB(i, R, G, B);
  }
}

void set_left(int NLed, int R, int G, int B){
if(NLed == NLED4x4){
  for (int i = 0; i < (NLED4x4); i++) {
    set_led_RGB(i, LED_PATTERN_DPC_LEFT_4x4[i]*R, LED_PATTERN_DPC_LEFT_4x4[i]*G, LED_PATTERN_DPC_LEFT_4x4[i]*B);
}}
if(NLed == NLED8x8){
  for (int i = 0; i < (NLED8x8); i++) {
    set_led_RGB(i, LED_PATTERN_DPC_LEFT_8x8[i]*R, LED_PATTERN_DPC_LEFT_8x8[i]*G, LED_PATTERN_DPC_LEFT_8x8[i]*B);
}}
}

void set_right(int NLed, int R, int G, int B){
if(NLed == NLED4x4){
  for (int i = 0; i < (NLED4x4); i++) {
    set_led_RGB(i, (1-LED_PATTERN_DPC_LEFT_4x4[i])*R, (1-LED_PATTERN_DPC_LEFT_4x4[i])*G, (1-LED_PATTERN_DPC_LEFT_4x4[i])*B);
}}
if(NLed == NLED8x8){
  for (int i = 0; i < (NLED8x8); i++) {
    set_led_RGB(i, (1-LED_PATTERN_DPC_LEFT_8x8[i])*R, (1-LED_PATTERN_DPC_LEFT_8x8[i])*G, (1-LED_PATTERN_DPC_LEFT_8x8[i])*B);
}}
}

void set_top(int NLed, int R, int G, int B){
if(NLed == NLED4x4){
  for (int i = 0; i < (NLED4x4); i++) {
    set_led_RGB(i, (LED_PATTERN_DPC_TOP_4x4[i])*R, (LED_PATTERN_DPC_TOP_4x4[i])*G, (LED_PATTERN_DPC_TOP_4x4[i])*B);
}}
if(NLed == NLED8x8){
  for (int i = 0; i < (NLED8x8); i++) {
    set_led_RGB(i, (LED_PATTERN_DPC_TOP_8x8[i])*R, (LED_PATTERN_DPC_TOP_8x8[i])*G, (LED_PATTERN_DPC_TOP_8x8[i])*B);
}}
}

void set_bottom(int NLed, int R, int G, int B){
if(NLed == NLED4x4){
  for (int i = 0; i < (NLED4x4); i++) {
    set_led_RGB(i, (1-LED_PATTERN_DPC_TOP_4x4[i])*R, (1-LED_PATTERN_DPC_TOP_4x4[i])*G, (1-LED_PATTERN_DPC_TOP_4x4[i])*B);
}}
if(NLed == NLED8x8){
  for (int i = 0; i < (NLED8x8); i++) {
    set_led_RGB(i, (1-LED_PATTERN_DPC_TOP_8x8[i])*R, (1-LED_PATTERN_DPC_TOP_8x8[i])*G, (1-LED_PATTERN_DPC_TOP_8x8[i])*B);
}}
}

void set_center(int R, int G, int B)
{
  /*
  matrix.fillScreen(matrix.Color(0, 0, 0));
  matrix.drawPixel(4, 4, matrix.Color(R,   G,   B));
  matrix.show();
  */
}

/*
   wrapper for HTTP requests
*/
void ledarr_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  ledarr_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void ledarr_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  ledarr_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void ledarr_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  ledarr_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
