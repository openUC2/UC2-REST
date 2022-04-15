#ifdef IS_LEDARR
#include "parameters_ledarr.h"


Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(LED_N_X, LED_N_Y, LED_ARRAY_PIN,
                            NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                            NEO_GRB            + NEO_KHZ800);


// Custom function accessible by the API
void ledarr_act_fct() {

  // here you can do something
  if (DEBUG) Serial.println("ledarr_act_fct");

  const char* LEDArrMode = jsonDocument["LEDArrMode"]; // "array", "full", "full", "single", "off", "left", "right", "top", "bottom",

  // individual pattern gets adressed
  // PYTHON: send_LEDMatrix_array(self, led_pattern, timeout=1)
  if (strcmp(LEDArrMode, "array")==0) {
    if (DEBUG) Serial.println("pattern");
    int arraySize = LED_N_X * LED_N_Y;
    if (jsonDocument.containsKey("arraySize")) {
      arraySize = jsonDocument["arraySize"];
    }
    for (int i = 0; i < arraySize; i++) { //Iterate through results
      int red = jsonDocument["red"][i];  //Implicit cast
      int green = jsonDocument["green"][i];  //Implicit cast
      int blue = jsonDocument["blue"][i];  //Implicit cast
      int ix = i % LED_N_X;
      int iy = i / LED_N_Y;
      set_led_RGB(ix, iy, red, green, blue);
    }
  }
  // only if a single led will be updated, all others stay the same
  // PYTHON: send_LEDMatrix_single(self, indexled=0, intensity=(255,255,255), timeout=1)
  else if (strcmp(LEDArrMode, "single")==0) {
    if (DEBUG) Serial.println("single");
      int indexled = jsonDocument["indexled"];
      int red = jsonDocument["red"];  //Implicit cast
      int green = jsonDocument["green"];  //Implicit cast
      int blue = jsonDocument["blue"];  //Implicit cast
      if (DEBUG) Serial.print(red); Serial.print(green); Serial.println(blue);
      int ix = indexled % LED_N_X;
      int iy = indexled / LED_N_Y;
      set_led_RGB(ix, iy, red, green, blue);
  }
  // only few leds will be updated, all others stay the same
  // PYTHON: send_LEDMatrix_multi(self, indexled=(0), intensity=((255,255,255)), Nleds=8*8, timeout=1)
  else if (strcmp(LEDArrMode, "multi")==0) {
    if (DEBUG) Serial.println("multi");
    int Nleds = jsonDocument["Nleds"];
    for (int i = 0; i < Nleds; i++) { //Iterate through results
      int indexled = jsonDocument["indexled"][i];
      int red = jsonDocument["red"][indexled];  //Implicit cast
      int green = jsonDocument["green"][indexled];  //Implicit cast
      int blue = jsonDocument["blue"][indexled];  //Implicit cast
      if (DEBUG) Serial.print(red); Serial.print(green); Serial.println(blue);
      int ix = indexled % LED_N_X;
      int iy = indexled / LED_N_Y;
      set_led_RGB(ix, iy, red, green, blue);
    }
  }
  // turn on all LEDs
  // PYTHON: send_LEDMatrix_full(self, intensity = (255,255,255),timeout=1)
  else if (strcmp(LEDArrMode, "full")==0) {
    if (DEBUG) Serial.println("full");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    set_all(red, green, blue);
  }
  // turn off all LEDs
  else if (strcmp(LEDArrMode, "left")==0) {
    if (DEBUG) Serial.println("left");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    set_left(red, green, blue);
  }
  // turn off all LEDs
  else if (strcmp(LEDArrMode, "right")==0) {
    if (DEBUG) Serial.println("right");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    set_right(red, green, blue);
  }
  // turn off all LEDs
  else if (strcmp(LEDArrMode, "top")==0) {
    if (DEBUG) Serial.println("top");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    set_top(red, green, blue);
  }
  // turn off all LEDs
  else if (strcmp(LEDArrMode, "bottom")==0) {
    if (DEBUG) Serial.println("bottom");
    int red = jsonDocument["red"];
    int green = jsonDocument["green"];
    int blue = jsonDocument["blue"];
    set_bottom(red, green, blue);
  }
  jsonDocument.clear();
  jsonDocument["return"] = 1;

}

void ledarr_set_fct() {
  if (jsonDocument["LED_ARRAY_PIN"] != 0) {
    //if (DEBUG) Serial.print("LED_ARRAY_PIN "); Serial.println(jsonDocument["LED_ARRAY_PIN"]);
    LED_ARRAY_PIN = jsonDocument["LED_ARRAY_PIN"];
  }

  if (jsonDocument["LED_N_X"] != 0) {
    //if (DEBUG) Serial.print("LED_N_X "); Serial.println(jsonDocument["LED_N_X"]);
    LED_N_X = jsonDocument["LED_N_X"];
  }

  if (jsonDocument["LED_N_Y"] != 0) {
    //if (DEBUG) Serial.print("LED_N_Y "); Serial.println(jsonDocument["LED_N_Y"]);
    LED_N_Y = jsonDocument["LED_N_Y"];
  }
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}



// Custom function accessible by the API
void ledarr_get_fct() {

  jsonDocument.clear();
  jsonDocument["LED_ARRAY_PIN"] = LED_ARRAY_PIN;
  jsonDocument["LED_N_X"] = LED_N_X;
  jsonDocument["LED_N_Y"] = LED_N_Y;
}



/***************************************************************************************************/
/*******************************************  LED Array  *******************************************/
/***************************************************************************************************/
/*******************************FROM OCTOPI ********************************************************/

void set_led_RGB(int Nx, int Ny, int R, int G, int B)  {
  matrix.drawPixel(Nx, Ny, matrix.Color(R,   G,   B));
  matrix.show();
}

void setup_matrix() {
  // LED Matrix
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(255);
  matrix.fillScreen(matrix.Color(0,0,255));
  matrix.show();
  //delay(1000);
  //matrix.fillScreen(0);
  //matrix.show();
}

void set_all(int R, int G, int B)
{
  for (int i = 0; i < (LED_N_X * LED_N_Y); i++) {
    int ix =  i % LED_N_X;
    int iy =  i / LED_N_Y;
    matrix.drawPixel(ix, iy, matrix.Color(R,   G,   B));
  if (DEBUG) Serial.print(R); Serial.print(G); Serial.println(B);
  }
  matrix.show();
}

void set_left(int R, int G, int B)
{
  for (int i = 0; i < (LED_N_X * LED_N_Y); i++) {
    int ix =  i % LED_N_X;
    int iy =  i / LED_N_Y;
    if(ix<LED_N_X/2){
      matrix.drawPixel(ix, iy, matrix.Color(R,   G,   B));
    }
    else{
      matrix.drawPixel(ix, iy, matrix.Color(0,0,0));
    }
  }
  matrix.show();
}

void set_right(int R, int G, int B)
{
  for (int i = 0; i > (LED_N_X * LED_N_Y); i++) {
    int ix =  i % LED_N_X;
    int iy =  i / LED_N_Y;
    if(ix<LED_N_X/2){
      matrix.drawPixel(ix, iy, matrix.Color(R,   G,   B));
    }
    else{
      matrix.drawPixel(ix, iy, matrix.Color(0,0,0));
    }
  }
  matrix.show();
}


void set_top(int R, int G, int B)
{
  for (int i = 0; i < (LED_N_X * LED_N_Y); i++) {
    int ix =  i % LED_N_X;
    int iy =  i / LED_N_Y;
    if(iy<LED_N_Y/2){
      matrix.drawPixel(ix, iy, matrix.Color(R,   G,   B));
    }
    else{
      matrix.drawPixel(ix, iy, matrix.Color(0,0,0));
    }
  }
  matrix.show();
}

void set_bottom(int R, int G, int B)
{
  for (int i = 0; i > (LED_N_X * LED_N_Y); i++) {
    int ix =  i % LED_N_X;
    int iy =  i / LED_N_Y;
    if(iy<LED_N_Y/2){
      matrix.drawPixel(ix, iy, matrix.Color(R,   G,   B));
    }
    else{
      matrix.drawPixel(ix, iy, matrix.Color(0,0,0));
    }
  }
  matrix.show();
}



/*
   wrapper for HTTP requests
*/

#ifdef IS_WIFI
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
#endif
#endif
