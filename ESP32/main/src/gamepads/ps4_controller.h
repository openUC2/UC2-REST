#ifndef ps4_controller_h
#define ps4_controller_h
#include "gamepad.h"
#include "PS4Controller.h"

class ps4_controller : public gamepad
{
    private:
    
    public:
    static PS4Controller::callback_t onAttached(gamepad * at);
    static PS4Controller::callback_t onConnected(gamepad * at);
    static PS4Controller::callback_t onDisConnected(gamepad * at);
    static PS4Controller::callback_t onActivated(gamepad * at);
};
#endif