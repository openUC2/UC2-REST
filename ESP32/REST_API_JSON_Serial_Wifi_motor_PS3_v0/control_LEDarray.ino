#ifdef IS_LEDARR
#include "parameters_ledarr.h"


Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(LED_N_X, LED_N_Y, LED_ARRAY_PIN,
                            NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                            NEO_GRB            + NEO_KHZ800);


// Custom function accessible by the API
void ledarr_act_fct() {
  if(DEBUG) Serial.println("ledarr_act_fct");

  int arraySize = LED_N_X*LED_N_Y;

  for (int i = 0; i < arraySize; i++) { //Iterate through results
    int red = jsonDocument["red"][i];  //Implicit cast
    int green = jsonDocument["green"][i];  //Implicit cast
    int blue = jsonDocument["blue"][i];  //Implicit cast
    if(DEBUG) Serial.print(red);Serial.print(green);Serial.println(blue);
    int ix = i%LED_N_X;
    int iy = i/LED_N_Y;
    set_led_RGB(ix, iy, red, green, blue);
  }

  jsonDocument.clear();
  jsonDocument["return"] = 1;

}

void ledarr_set_fct() {


  if (jsonDocument["LED_ARRAY_PIN"] != 0) {
    //if (DEBUG) Serial.print("LED_ARRAY_PIN "); Serial.println(jsonDocument["LED_ARRAY_PIN"]);
    LED_ARRAY_PIN= jsonDocument["LED_ARRAY_PIN"];
  }

  if (jsonDocument["LED_N_X"] != 0) {
    //if (DEBUG) Serial.print("LED_N_X "); Serial.println(jsonDocument["LED_N_X"]);
    LED_N_X= jsonDocument["LED_N_X"];
  }

  if (jsonDocument["LED_N_Y"] != 0) {
    //if (DEBUG) Serial.print("LED_N_Y "); Serial.println(jsonDocument["LED_N_Y"]);
    LED_N_Y= jsonDocument["LED_N_Y"];
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




void setup_matrix(){
  // LED Matrix
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(255);
  matrix.fillScreen(255);
  matrix.show();
  delay(1000);
  matrix.fillScreen(0);
  matrix.show();
}



void set_led_RGB(int Nx, int Ny, int R, int G, int B)  {
  matrix.drawPixel(Nx, Ny, matrix.Color(R,   G,   B));
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
