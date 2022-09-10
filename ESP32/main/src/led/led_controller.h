#ifndef led_controller_h
#define led_controller_h

#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <ArduinoJson.h>
#include "../../pinstruct.h"

class led_controller
{
    public:
    led_controller();
    ~led_controller();
    bool DEBUG = false;
    bool isBusy;

    int NLED4x4=16;
    int NLED8x8=64;

    int LED_PATTERN_DPC_TOP_8x8 [64] = {1,1,1,1,1,1,1,1,
                                        1,1,1,1,1,1,1,1,
                                        1,1,1,1,1,1,1,1,
                                        1,1,1,1,1,1,1,1,      
                                        0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0};

    int LED_PATTERN_DPC_LEFT_8x8 [64]= {1,1,1,1,0,0,0,0,
                                        0,0,0,0,1,1,1,1,
                                        1,1,1,1,0,0,0,0,
                                        0,0,0,0,1,1,1,1,
                                        1,1,1,1,0,0,0,0,
                                        0,0,0,0,1,1,1,1,
                                        1,1,1,1,0,0,0,0,
                                        0,0,0,0,1,1,1,1};
                                  
    int LED_PATTERN_DPC_TOP_4x4 [16]=  {1,1,1,1,
                                        1,1,1,1,
                                        0,0,0,0,
                                        0,0,0,0};

    int LED_PATTERN_DPC_LEFT_4x4 [16]= {1,1,0,0,
                                        0,0,1,1,
                                        1,1,0,0,
                                        0,0,1,1};

    void act();
    void set();
    void get();
    void set_led_RGB(int iLed, int R, int G, int B);
    void setup_matrix();
    void set_all(int R, int G, int B);
    void set_left(int NLed, int R, int G, int B);
    void set_right(int NLed, int R, int G, int B);
    void set_top(int NLed, int R, int G, int B);
    void set_bottom(int NLed, int R, int G, int B);
    void set_center(int R, int G, int B);

    // We use the strip instead of the matrix to ensure different dimensions; Convesion of the pattern has to be done on the cliet side!
    Adafruit_NeoPixel * matrix;
    DynamicJsonDocument * jsonDocument;
    PINDEF * pins;

};
static led_controller led;
#endif