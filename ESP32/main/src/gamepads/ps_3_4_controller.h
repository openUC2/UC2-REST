#pragma once
#include "../../config.h"
#ifdef IS_MOTOR
    #include "../motor/FocusMotor.h"
#endif
#ifdef IS_LED
#include "../led/LedController.h"
#endif
    #include "../state/State.h"
#ifdef IS_LASER
    #include "../laser/LaserController.h"
#endif
#ifdef IS_ANALOG
    #include "../analog/AnalogController.h"
#endif
#ifdef IS_PS4
    #include "PS4Controller.h"
#endif
#ifdef IS_PS3
    #include "Ps3Controller.h"
#endif



class ps_3_4_controller
{
    private:
    /* data */
        int is_share();
        int is_circle();
        int is_triangle();
        int is_cross();
        int is_square();
        int is_down_down();
        int is_down_up();
        int is_down_right();
        int is_down_left();
        int is_charging();
        int is_connected();
        int analog_ly();
        int analog_lx();
        int analog_ry();
        int analog_rx();
        int left();
        int right();
        //int start_b();
        int r2();
        int l2();
        int l1();
        int r1();
        

    protected:
    public:
        ps_3_4_controller();
        ~ps_3_4_controller();

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

        int analog_val_1 = 0;
        int pwm_max = 0; // no idea how big it should be

        bool DEBUG = false;
        bool IS_PS_CONTROLER_LEDARRAY = false;
        virtual void start();
        void stop();

        void onConnect();
        void onAttach();
        void onDisConnect();
        void activate();
        void control();
        
        static void ps_onAttach(void *ob);
        static void ps_onConnect(void * parameter);
        static void ps_onDisConnect(void * parameter);
        static void ps_activate(void * parameter);
};
static ps_3_4_controller ps_c;