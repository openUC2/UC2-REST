#ifndef DacController_h
#define DacController_h

#include <ArduinoJson.h>
#include "DAC_Module.h"
#include "../../pinstruct.h"

class DacController
{
    private:
    #ifdef IS_DAC
    DAC_Module * dac;
    #endif

    public:
    DacController();
    ~DacController();
    bool DEBUG = false;
    DynamicJsonDocument * jsonDocument;
    PINDEF * pins;

    // DAC-specific parameters
    dac_channel_t dac_channel = DAC_CHANNEL_1;

    uint32_t clk_div = 0;
    uint32_t scale = 0;
    uint32_t invert = 2;
    uint32_t phase = 0;

    uint32_t frequency = 1000;

    boolean dac_is_running = false;

    void setup();

    void dac_act_fct();
    void dac_set_fct();
    void dac_get_fct();

    #ifdef IS_WIFI
    void dac_act_fct_http();
    void dac_get_fct_http();
    void dac_set_fct_http();
    void drive_galvo(void * parameter);
    #endif
};

#endif