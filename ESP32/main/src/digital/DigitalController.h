#ifndef DigitalController_h
#define DigitalController_h

#include <ArduinoJson.h>
#include "../../pinstruct.h"

class DigitalController
{
private:
    /* data */
public:
    DigitalController(/* args */);
    ~DigitalController();

    bool isBusy;
    bool DEBUG = false;
    PINDEF * pins;

    int digital_val_1 = 0;
    int digital_val_2 = 0;
    int digital_val_3 = 0;

    void digital_act_fct(int digitalid,int digitalval);
    void digital_set_fct(int digitalid,int digitalpin);
    void digital_get_fct(DynamicJsonDocument * jsonDocument);
    void setupDigital();
};

DigitalController::DigitalController(/* args */)
{
}

DigitalController::~DigitalController()
{
}


#endif