#ifndef GAMEDPAD_H
#define GAMEDPAD_H

#include "../motor/FocusMotor.h"
#include "../led/led_controller.h"

class gamepad
{
private:
    /* data */
protected:
public:
    gamepad();
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
    void start();
    void stop();

    void onConnect();
    void onAttach();
    void onDisConnect();
    void activate();
    void control();
};

gamepad * gamepad_controller;

void gamepad_onAttach()
{
    gamepad_controller->onAttach();
}

void gamepad_onConnect()
{
    gamepad_controller->onConnect();
}

void gamepad_onDisConnect()
{
    gamepad_controller->onDisConnect();
}

void gamepad_activate()
{
    gamepad_controller->activate();
}

#endif