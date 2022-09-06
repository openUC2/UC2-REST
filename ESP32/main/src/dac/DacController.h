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

    void act();
    void set();
    void get();

   
};

DacController::DacController()
{
  #ifdef IS_DAC
    dac = new DAC_Module();
  #endif
}

DacController::~DacController()
{
    
}

DacController * dac;

#endif