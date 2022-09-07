#ifdef IS_PS4
#ifndef ps4_controller_h
#define ps4_controller_h
#include "PS4Controller.h"

class ps4_controller
{
    private:
    /* data */
    protected:
    public:
        ps4_controller();
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

static ps4_controller * ps4_c;

static void ps4_onAttach()
{
    ps4_c->onAttach();
};
static void ps4_onConnect(){
    ps4_c->onConnect();
};
static void ps4_onDisConnect(){
    ps4_c->onDisConnect();
};
static void ps4_activate(){
    ps4_c->activate();
};
#endif
#endif