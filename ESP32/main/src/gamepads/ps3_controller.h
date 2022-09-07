#ifdef IS_PS3
#ifndef ps3_controller_h
#define ps3_controller_h

#include <Ps3Controller.h>
#include "../motor/FocusMotor.h"
#include "../state/State.h"
#include "../laser/LaserController.h"
#include "../analog/AnalogController.h"
#include "../led/led_controller.h"

class ps3_controller
{
    private:
    /* data */
    protected:
    public:
        ps3_controller();
        int offset_val = 20; // make sure you do not accidentally turn on two directions at the same time
        int stick_ly = 0;
        int stick_lx = 0;
        int stick_rx = 0;
        int stick_ry = 0;

        int speed_x = 0;
        int speed_y = 0;
        int speed_z = 0;

        int IS_PSCONTROLER_ACTIVE = false;

        int global_speed = 2; // multiplier for the speed

        bool DEBUG = false;
        bool IS_PS_CONTROLER_LEDARRAY = false;
        virtual void start();
        void stop();

        void onConnect();
        void onAttach();
        void onDisConnect();
        void activate();
        void control();
};

static ps3_controller * ps3_c;

static void ps3_onAttach()
{
    ps3_c->onAttach();
};
static void ps3_onConnect(){
    ps3_c->onConnect();
};
static void ps3_onDisConnect(){
    ps3_c->onDisConnect();
};
static void ps3_activate(){
    ps3_c->activate();
};

#endif
#endif