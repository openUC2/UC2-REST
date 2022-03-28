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

  const char* LEDArrMode = jsonDocument["LEDArrMode"]; // "array", "individual", "full", "off", "left", "right", "top", "bottom",

  // individual pattern gets adressed
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
      if (DEBUG) Serial.print(red); Serial.print(green); Serial.println(blue);
      int ix = i % LED_N_X;
      int iy = i / LED_N_Y;
      set_led_RGB(ix, iy, red, green, blue);
    }
  }

  // only few leds will be updated, all others stay the same
  else if (strcmp(LEDArrMode, "individual")==0) {
    if (DEBUG) Serial.println("individual");
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
    int red = 0;
    int green = 0;
    int blue = 0;
    set_all(red, green, blue);
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




void setup_matrix() {
  // LED Matrix
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(255);
  matrix.fillScreen((255,255,255));
  matrix.show();
  //delay(1000);
  //matrix.fillScreen(0);
  //matrix.show();
}



void set_led_RGB(int Nx, int Ny, int R, int G, int B)  {
  matrix.drawPixel(Nx, Ny, matrix.Color(R,   G,   B));
  matrix.show();
}



/***************************************************************************************************/
/*******************************************  LED Array  *******************************************/
/***************************************************************************************************/
/*******************************FROM OCTOPI ********************************************************/
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
/*
  void set_left(int r, int g, int b)
  {
  for (int i = 0; i < LED_N_X/2; i++)
    matrix.setPixelColor(i,r,g,b);
  }

  void set_right(Adafruit_DotStar & matrix, int r, int g, int b)
  {
  for (int i = DOTSTAR_NUM_LEDS/2; i < DOTSTAR_NUM_LEDS; i++)
    matrix.setPixelColor(i,r,g,b);
  }

  void set_low_na(Adafruit_DotStar & matrix, int r, int g, int b)
  {
  // matrix.setPixelColor(44,r,g,b);
  matrix.setPixelColor(45,r,g,b);
  matrix.setPixelColor(46,r,g,b);
  // matrix.setPixelColor(47,r,g,b);
  matrix.setPixelColor(56,r,g,b);
  matrix.setPixelColor(57,r,g,b);
  matrix.setPixelColor(58,r,g,b);
  matrix.setPixelColor(59,r,g,b);
  matrix.setPixelColor(68,r,g,b);
  matrix.setPixelColor(69,r,g,b);
  matrix.setPixelColor(70,r,g,b);
  matrix.setPixelColor(71,r,g,b);
  // matrix.setPixelColor(80,r,g,b);
  matrix.setPixelColor(81,r,g,b);
  matrix.setPixelColor(82,r,g,b);
  // matrix.setPixelColor(83,r,g,b);
  }

  void set_left_dot(Adafruit_DotStar & matrix, int r, int g, int b)
  {
  matrix.setPixelColor(3,r,g,b);
  matrix.setPixelColor(4,r,g,b);
  matrix.setPixelColor(11,r,g,b);
  matrix.setPixelColor(12,r,g,b);
  }

  void set_right_dot(Adafruit_DotStar & matrix, int r, int g, int b)
  {
  matrix.setPixelColor(115,r,g,b);
  matrix.setPixelColor(116,r,g,b);
  matrix.setPixelColor(123,r,g,b);
  matrix.setPixelColor(124,r,g,b);
  }

  void clear_matrix(Adafruit_DotStar & matrix)
  {
  for (int i = 0; i < DOTSTAR_NUM_LEDS; i++)
    matrix.setPixelColor(i, 0, 0, 0);
  matrix.show();
  }
*/


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
