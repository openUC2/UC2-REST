#pragma once
#include "../../config.h"
#include <JPEGDecoder.h>
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <ArduinoJson.h>
#include "../wifi/WifiController.h"

// Return the minimum of two values a and b
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

#define TFT_RST    -1    // we use the seesaw for resetting to save a pin

#ifdef ESP8266
   #define TFT_CS   0
   #define TFT_DC   15
#elif defined(ESP32)
   #define TFT_CS   15
   #define TFT_DC   33
#elif defined(TEENSYDUINO)
   #define TFT_DC   10
   #define TFT_CS   4
#elif defined(ARDUINO_STM32_FEATHER)
   #define TFT_DC   PB4
   #define TFT_CS   PA15
#elif defined(ARDUINO_NRF52832_FEATHER)  /* BSP 0.6.5 and higher! */
   #define TFT_DC   11
   #define TFT_CS   31
#elif defined(ARDUINO_MAX32620FTHR) || defined(ARDUINO_MAX32630FTHR)
   #define TFT_DC   P5_4
   #define TFT_CS   P5_3
#else
    // Anything else, defaults!
   #define TFT_CS   9
   #define TFT_DC   10
#endif

/**
 * control Adafruit_ST7735 tft display with json
 */
class SlmController
{
private:
    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, /*TFT_MOSI, TFT_CLK,*/ TFT_RST);
    int NX = tft.height();
    int NY = tft.width();
public:
    SlmController(/* args */);
    ~SlmController();
    bool DEBUG = false;
    bool isBusy;

    void act();
    void set();
    void get();
    void setup();

    void createArray(const char *filename);
    void jpegInfo();
    void jpegRender(int xpos, int ypos);
    void drawJpeg(String filename, int xpos, int ypos);
};

/**
 * control Adafruit_ST7735 tft display with json
 */
static SlmController slm;