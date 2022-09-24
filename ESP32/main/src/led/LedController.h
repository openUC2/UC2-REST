#pragma once
#include "../../config.h"
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include "../../pinstruct.h"
#include "../config/JsonKeys.h"
#include "../wifi/WifiController.h"

namespace LedController
{
    void act();
    void set();
    void get();
    void set_led_RGB(u_int8_t iLed, u_int8_t R, u_int8_t G, u_int8_t B);
    void setup(PINDEF * pins,bool debug);
    void set_all(u_int8_t R, u_int8_t G, u_int8_t B);
    void set_left(u_int8_t NLed, u_int8_t R, u_int8_t G, u_int8_t B);
    void set_right(u_int8_t NLed, u_int8_t R, u_int8_t G, u_int8_t B);
    void set_top(u_int8_t NLed, u_int8_t R, u_int8_t G, u_int8_t B);
    void set_bottom(u_int8_t NLed, u_int8_t R, u_int8_t G, u_int8_t B);
    void set_center(u_int8_t R, u_int8_t G, u_int8_t B);
}