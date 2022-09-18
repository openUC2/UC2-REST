#pragma once
#include "../../config.h"
#include "AccelStepper.h"
#include <ArduinoJson.h>
#include "../../pinstruct.h"
#if defined IS_PS3 || defined IS_PS4
    #include "../gamepads/ps_3_4_controller.h"
#endif
#include "../wifi/WifiController.h"

class FocusMotor
{
    public:
        FocusMotor();
        ~FocusMotor();

        PINDEF * pins;

        bool DEBUG = false;

        // for stepper.h
        #define MOTOR_STEPS 200
        #define SLEEP 0
        #define MS1 0
        #define MS2 0
        #define MS3 0
        #define RPM 120

        bool isaccel = false;
        bool isforever = false;
        bool motor_enable = false;
        bool isBusy = false;

        // global variables for the motor
        long mspeed0 = 1000;
        long mspeed1 = 1000;
        long mspeed2 = 1000;
        long mspeed3 = 1000;
        long mposition0 = 0;
        long mposition1 = 0;
        long mposition2 = 0;
        long mposition3 = 0;
        boolean isstop = 0;

        long POSITION_MOTOR_A = 0;
        long POSITION_MOTOR_X = 0;
        long POSITION_MOTOR_Y = 0;
        long POSITION_MOTOR_Z = 0;

        int MOTOR_ACCEL = 5000;
        int MOTOR_DECEL = 5000;
        
        int isabs = true;
        int isen = false;
        bool isactive = false;

        // direction
        int SIGN_A = 1;
        int SIGN_X = 1;
        int SIGN_Y = 1;
        int SIGN_Z = 1;
        static const int FULLSTEPS_PER_REV_A = 200;
        static const int FULLSTEPS_PER_REV_X = 200;
        static const int FULLSTEPS_PER_REV_Y = 200;
        static const int FULLSTEPS_PER_REV_Z = 200;

        long MAX_VELOCITY_A = 20000;
        long MAX_VELOCITY_X = 20000;
        long MAX_VELOCITY_Y = 20000;
        long MAX_VELOCITY_Z = 20000;
        long MAX_ACCELERATION_A = 100000;
        long MAX_ACCELERATION_X = 100000;
        long MAX_ACCELERATION_Y = 100000;
        long MAX_ACCELERATION_Z = 100000;


        AccelStepper * stepper_A;
        AccelStepper * stepper_X;
        AccelStepper * stepper_Y;
        AccelStepper * stepper_Z;


        void act();
        void setEnableMotor(bool enable);
        bool getEnableMotor();
        void set();
        void get();
        void setup(PINDEF * pins);
        bool background();

};

static FocusMotor motor;