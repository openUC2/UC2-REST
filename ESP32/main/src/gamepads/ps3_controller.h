#ifndef ps3_controller_h
#define ps3_controller_h

#include "gamepad.h"

class ps3_controller : public gamepad
{
    public:
    static Ps3Controller::callback_t onAttached(gamepad * at);
    static Ps3Controller::callback_t onConnected(gamepad * at);
    static Ps3Controller::callback_t onDisConnected(gamepad * at);
    static Ps3Controller::callback_t onActivated(gamepad * at);
};
#endif