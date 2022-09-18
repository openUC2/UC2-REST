#include "led_controller.h"


led_controller::~led_controller(){
  matrix = nullptr;
};

led_controller::led_controller()
{
}

// Custom function accessible by the API
void led_controller::act() {

  // here you can do something
  if (DEBUG) Serial.println(F("ledarr_act_fct"));
  //TODO: figure out what its blocking
  //isBusy = true;

  JsonObject ledNode = (*WifiController::getJDoc())[keyLed];

  LedModes LEDArrMode = ledNode[keyLEDArrMode]; // "array", "full", "single", "off", "left", "right", "top", "bottom",
  int NLeds = ledNode[keyNLeds];
  JsonArray ledcolorArray = ledNode[F("led_array")];
  if (DEBUG) Serial.println(LEDArrMode);
  // individual pattern gets adressed
  // PYTHON: send_LEDMatrix_array(self, led_pattern, timeout=1)
  if (LEDArrMode == LedModes::array || LEDArrMode == LedModes::multi) {
    for(int i = 0; i < ledcolorArray.size(); i++)
    {
      JsonObject cobj = ledcolorArray[i];
      set_led_RGB(cobj[keyid], cobj[keyRed],cobj[keyGreen],cobj[keyBlue]);
    }
  }
  // only if a single led will be updated, all others stay the same
  // PYTHON: send_LEDMatrix_single(self, indexled=0, intensity=(255,255,255), timeout=1)
  else if (LEDArrMode == LedModes::single) {
      JsonObject cobj = ledcolorArray[0];
      set_led_RGB(cobj[keyid], cobj[keyRed],cobj[keyGreen],cobj[keyBlue]);
  }
  // turn on all LEDs
  // PYTHON: send_LEDMatrix_full(self, intensity = (255,255,255),timeout=1)
  else if (LEDArrMode == LedModes::full) {
      JsonObject cobj = ledcolorArray[0];
      set_all(cobj[keyRed],cobj[keyGreen],cobj[keyBlue]);
  }
  // turn off all LEDs
  else if (LEDArrMode == LedModes::left) {
    JsonObject cobj = ledcolorArray[0];
    set_left(NLeds, cobj[keyRed],cobj[keyGreen],cobj[keyBlue]);
  }
  // turn off all LEDs
  else if (LEDArrMode == LedModes::right) {
    JsonObject cobj = ledcolorArray[0];
    set_right(NLeds,cobj[keyRed],cobj[keyGreen],cobj[keyBlue]);
  }
  // turn off all LEDs
  else if (LEDArrMode == LedModes::top) {
    JsonObject cobj = ledcolorArray[0];
    set_top(NLeds, cobj[keyRed],cobj[keyGreen],cobj[keyBlue]);
  }
  // turn off all LEDs
  else if (LEDArrMode == LedModes::bottom) {
    JsonObject cobj = ledcolorArray[0];
    set_bottom(NLeds, cobj[keyRed],cobj[keyGreen],cobj[keyBlue]);
  }
  else if (LEDArrMode == LedModes::bottom) {
    matrix->clear();
  }
  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())[F("return")] = 1;
  (*WifiController::getJDoc())[keyLEDArrMode] = LEDArrMode;
  isBusy = false;

}

void led_controller::set() {

  Serial.println(F("Updating Hardware config of LED Array"));

  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())[F("return")] = 1;
}



// Custom function accessible by the API
void led_controller::get() {
  Serial.print(F("led_controller::get() jsondoc null "));
  Serial.println(WifiController::getJDoc() == nullptr);
  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())[keyNLeds] = pins->LED_ARRAY_NUM;
  (*WifiController::getJDoc())[keyLEDArrMode].add(0);
  (*WifiController::getJDoc())[keyLEDArrMode].add(1);
  (*WifiController::getJDoc())[keyLEDArrMode].add(2);
  (*WifiController::getJDoc())[keyLEDArrMode].add(3);
  (*WifiController::getJDoc())[keyLEDArrMode].add(4);
  (*WifiController::getJDoc())[keyLEDArrMode].add(5);
  (*WifiController::getJDoc())[keyLEDArrMode].add(6);
  (*WifiController::getJDoc())[keyLEDArrMode].add(7);
  //(*jsonDocument)[F("LED_ARRAY_PIN")] = pins->LED_ARRAY_PIN;
}



/***************************************************************************************************/
/*******************************************  LED Array  *******************************************/
/***************************************************************************************************/
/*******************************FROM OCTOPI ********************************************************/

void led_controller::set_led_RGB(int iLed, int R, int G, int B)  {
  matrix->setPixelColor(iLed, matrix->Color(R,   G,   B));         //  Set pixel's color (in RAM)
  matrix->show();                          //  Update strip to match
}

void led_controller::setup(PINDEF * pi) {
  // LED Matrix
  pins = pi;
  matrix = new Adafruit_NeoPixel(pins->LED_ARRAY_NUM, pins->LED_ARRAY_PIN, NEO_GRB + NEO_KHZ800);
  if(DEBUG) Serial.println(F("Setting up LED array"));
  if(DEBUG){ Serial.print(F("LED_ARRAY_PIN: "));  Serial.println(pins->LED_ARRAY_PIN);}
  matrix->begin();
  matrix->setBrightness(255);
  set_all(0,0,0);
  delay(100);
  set_all(100,100,100);
}

void led_controller::set_all(int R, int G, int B)
{
  for (int i = 0; i < (pins->LED_ARRAY_NUM); i++) {
    set_led_RGB(i, R, G, B);
  }
}

void led_controller::set_left(int NLed, int R, int G, int B){
    if(NLed == NLED4x4){
        for (int i = 0; i < (NLED4x4); i++) {
            set_led_RGB(i, LED_PATTERN_DPC_LEFT_4x4[i]*R, LED_PATTERN_DPC_LEFT_4x4[i]*G, LED_PATTERN_DPC_LEFT_4x4[i]*B);
        }
    }
    if(NLed == NLED8x8){
        for (int i = 0; i < (NLED8x8); i++) {
            set_led_RGB(i, LED_PATTERN_DPC_LEFT_8x8[i]*R, LED_PATTERN_DPC_LEFT_8x8[i]*G, LED_PATTERN_DPC_LEFT_8x8[i]*B);
        }
    }
}

void led_controller::set_right(int NLed, int R, int G, int B){
    if(NLed == NLED4x4){
        for (int i = 0; i < (NLED4x4); i++) {
            set_led_RGB(i, (1-LED_PATTERN_DPC_LEFT_4x4[i])*R, (1-LED_PATTERN_DPC_LEFT_4x4[i])*G, (1-LED_PATTERN_DPC_LEFT_4x4[i])*B);
        }
    }
    if(NLed == NLED8x8){
        for (int i = 0; i < (NLED8x8); i++) {
            set_led_RGB(i, (1-LED_PATTERN_DPC_LEFT_8x8[i])*R, (1-LED_PATTERN_DPC_LEFT_8x8[i])*G, (1-LED_PATTERN_DPC_LEFT_8x8[i])*B);
        }
    }
}

void led_controller::set_top(int NLed, int R, int G, int B){
    if(NLed == NLED4x4){
        for (int i = 0; i < (NLED4x4); i++) {
        set_led_RGB(i, (LED_PATTERN_DPC_TOP_4x4[i])*R, (LED_PATTERN_DPC_TOP_4x4[i])*G, (LED_PATTERN_DPC_TOP_4x4[i])*B);
        }
    }
    if(NLed == NLED8x8){
        for (int i = 0; i < (NLED8x8); i++) {
            set_led_RGB(i, (LED_PATTERN_DPC_TOP_8x8[i])*R, (LED_PATTERN_DPC_TOP_8x8[i])*G, (LED_PATTERN_DPC_TOP_8x8[i])*B);
        }
    }
}

void led_controller::set_bottom(int NLed, int R, int G, int B){
    if(NLed == NLED4x4){
        for (int i = 0; i < (NLED4x4); i++) {
            set_led_RGB(i, (1-LED_PATTERN_DPC_TOP_4x4[i])*R, (1-LED_PATTERN_DPC_TOP_4x4[i])*G, (1-LED_PATTERN_DPC_TOP_4x4[i])*B);
        }
    }
    if(NLed == NLED8x8){
        for (int i = 0; i < (NLED8x8); i++) {
            set_led_RGB(i, (1-LED_PATTERN_DPC_TOP_8x8[i])*R, (1-LED_PATTERN_DPC_TOP_8x8[i])*G, (1-LED_PATTERN_DPC_TOP_8x8[i])*B);
        }
    }
}

void led_controller::set_center(int R, int G, int B)
{
  /*
  matrix.fillScreen(matrix.Color(0, 0, 0));
  matrix.drawPixel(4, 4, matrix.Color(R,   G,   B));
  matrix.show();
  */
}