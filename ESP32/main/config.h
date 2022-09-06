#include "pinstruct.h"
int DEBUG = 1; // if tihs is set to true, the arduino runs into problems during multiple serial prints..
#define BAUDRATE 115200

//#define IS_PS3
#define IS_PS4
//#define DEBUG_GAMEPAD
//#define IS_ANALOG

#define IS_WIFI
#define IS_DAC
#define IS_DAC_FAKE
#define IS_READSENSOR
#define IS_PID
#define IS_LASER
#define IS_ANALOG
#define IS_DIGITAL
#define IS_SCANNER

#define IS_SERIAL

//#define DEBUG_MOTOR
//#define DEBUG_LED
//#define DEBUG_LASER

//#define DEBUG_ANALOG



//pindefs
//#define cellSTORM_cellphone
//#define pindef_cellstorm_wemos
//#define cellSTORM_WIFI
//#define cellSTORM
//#define cnc_shield_esp
//#define confocal
#define freedcam //killerink use that!
//#define incubator_microscope_zonly_matrix
//#define light_sheet_arduino
//#define light_sheet_esp_tomo
//#define lightsheet_esp_wemos
//#define lightsheet_tomo_galvo_espwemos
//#define lightsheet_tomo_galvo
//#define lightsheet_tomo_PID_espwemos
//#define lightsheet
//#define multicolor_borstel
//#define multicolour_fluorescence_wemos_borstel
//#define multicolour_wemos_lena
//#define multicolour
//#define oct_eda
//#define ptychography
//#define slm
//#define uc2standalone
//#define xyz_stagescan_ps4




void getDefaultPinDef(PINDEF pindef)
{
    #ifdef cellSTORM_cellphone
        #define IS_ANALOG// ESP32-only
        pindef.identifier_setup = "cellSTORM_cellphone"; 
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 21;
        pindef.analog_PIN_2 = 22;
        pindef.analog_PIN_3 = 0;
        // Definition cellSTORM
        pindef.STEP_X = 32;
        pindef.STEP_Y = 25;
        pindef.STEP_Z = 2;
        pindef.DIR_X = 23;
        pindef.DIR_Y = 23;
        pindef.DIR_Z = 23;
        pindef.ENABLE = 19;
        // Laser PWM pins
        pindef.LASER_PIN_1 = 27;
        pindef.LASER_PIN_2 = 0;
        pindef.LASER_PIN_3 = 0;
        #define PIN_ENABLE 26
    #endif
    #ifdef pindef_cellstorm_wemos
        #include "pindef_WEMOS_d1_r32.h"
        // ESP32-only
        #define IS_DIGITAL
        //#define IS_DAC
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 0;
        pindef.analog_PIN_2 = 0;
        pindef.analog_PIN_3 = 0;
        // Stepper Motor pins
        pindef.STEP_A = 0;
        pindef.STEP_X = X_STEP_PIN;
        pindef.STEP_Y = Y_STEP_PIN;
        pindef.STEP_Z = Z_STEP_PIN;
        ipindef.DIR_A = 0;
        pindef.DIR_X = X_DIRECTION_PIN;
        pindef.DIR_Y = Y_DIRECTION_PIN;
        pindef.DIR_Z = Z_DIRECTION_PIN;
        pindef.ENABLE = STEPPERS_ENABLE_PIN;
        // Laser PWM pins
        pindef.LASER_PIN_1 = SPINDLEPWMPIN; // Spin Dir
        pindef.LASER_PIN_2 = SPINDLE_ENABLE_PIN;//  Spin En
        pindef.LASER_PIN_3 = FEED_HOLD_PIN;//X_END_STOP;//
        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 0;
        pindef.digital_PIN_2 = 0;
        pindef.digital_PIN_3 = 0;
        pindef.identifier_setup = "pindef_cellstorm_wemos";
    #endif
    #ifdef cellSTORM_WIFI
        //#define IS_WIFI
        // // ESP32-only
        //#define IS_ANALOG// ESP32-only
        #define IS_DIGITAL
        //#define IS_DAC
        pindef.mSSID = "Blynk"; //"BenMur"; //
        pindef.mPWD = "12345678"; // "MurBen3128";//
        pindef.identifier_setup = "cellSTORM"; 
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 25;
        pindef.analog_PIN_2 = 26;
        pindef.analog_PIN_3 = 0;
        // Definition cellSTORM
        pindef.STEP_X = 21;
        pindef.STEP_Y = 22;
        pindef.STEP_Z = 23;
        pindef.DIR_X = 18;
        pindef.DIR_Y = 18;
        pindef.DIR_Z = 18;
        pindef.ENABLE = 19;
        // Laser PWM pins
        pindef.LASER_PIN_1 = 27;
        pindef.LASER_PIN_2 = 0;
        pindef.LASER_PIN_3 = 0;
        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 12; 
        pindef.digital_PIN_2 = 13; 
        pindef.digital_PIN_3 = 0; 
    #endif
    #ifdef cELLSTORM
            //#define IS_WIFI
        // ESP32-only
        #define IS_ANALOG// ESP32-only
        //#define IS_DAC
        pindef.identifier_setup = "cellSTORM"; 
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 25;
        pindef.analog_PIN_2 = 26;
        pindef.analog_PIN_3 = 0;
        // Definition cellSTORM
        pindef.STEP_X = 21;
        pindef.STEP_Y = 22;
        pindef.STEP_Z = 23;
        pindef.DIR_X = 18;
        pindef.DIR_Y = 18;
        pindef.DIR_Z = 18;
        pindef.ENABLE = 19;
        // Laser PWM pins
        pindef.LASER_PIN_1 = 27;
        pindef.LASER_PIN_2 = 0;
        pindef.LASER_PIN_3 = 0;
        #define PIN_ENABLE 19
    #endif

    #ifdef cnc_shield_esp
        //#define IS_WIFI
        // // ESP32-only
        //#define IS_ANALOG// ESP32-only
        #define IS_DIGITAL
        //#define IS_DAC
        pindef.mSSID = "Blynk"; //"BenMur"; //
        pindef.mPWD = "12345678"; // "MurBen3128";//
        pindef.identifier_setup = "ESP32_CNC_SHIELD"; 
        /*
        Ard - ESP - CNC
        0 -RX
        1 TX
        2 - 26 - xstep
        3 - 25 - ystep 
        4 17 - zstepp
        5 16 - xdir
        6 27 - ydir
        7 14 - zdir
        8 12 - EN
        9 13 - X-Endstop
        10 5 - Y-Endstop
        11 23 - Z-Endstop
        12 19 - SpinEnable
        13 18 - SpinDir

        A0 - 2
        A1 - 4
        A2 - 35
        A3 - 34. Coolant
        A4 - 36 -  A4 - 
        A5 - 39 -  A5 - 
        */
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 0;
        pindef.analog_PIN_2 = 0;
        pindef.analog_PIN_3 = 0;
        // Definition cellSTORM
        pindef.STEP_X = 26;
        pindef.STEP_Y = 25;
        pindef.STEP_Z = 17;
        pindef.DIR_X = 16;
        pindef.DIR_Y = 27;
        pindef.DIR_Z = 14;
        pindef.ENABLE = 12;
        // Laser PWM pins
        pindef.LASER_PIN_1 = 13;
        pindef.LASER_PIN_2 = 5;
        pindef.LASER_PIN_3 = 23;
        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 36; 
        pindef.digital_PIN_2 = 39; 
        pindef.digital_PIN_3 = 34; 
    #endif

    #ifdef confocal
        #include "pindef_WEMOS_d1_r32.h"
        #define IS_SCANNER
        // ESP32-only

        // Laser PWM pins for CNC Shield
        pindef.LASER_PIN_1 = 0; // was SPINDLEPWMPIN; // Spin Dir
        pindef.LASER_PIN_2 = Y_LIMIT_PIN; // was SPINDLE_ENABLE_PIN;//  Spin En
        pindef.LASER_PIN_3 = 0;// 

        // Stepper Motor pins
        pindef.STEP_A = 0;
        pindef.STEP_X = 0;
        pindef.STEP_Y = 0;
        pindef.STEP_Z = Z_STEP_PIN;
        pindef.DIR_A = 0;
        pindef.DIR_X = 0;
        pindef.DIR_Y = 0;
        pindef.DIR_Z = Z_DIRECTION_PIN;
        pindef.ENABLE = STEPPERS_ENABLE_PIN;

        // GALVos are always connected to 25/26 
        pindef.dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
        pindef.dac_fake_2 = 0; // Coolant

        // ledarray
        pindef.LED_ARRAY_PIN = X_LIMIT_PIN; // was FEED_HOLD_PIN; // FEED HOLD 

        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 0; //  Cycle Start/Resume
        pindef.digital_PIN_2 = 0; //not used/reserved
        pindef.digital_PIN_3 = 0; //not used/reserved

        pindef.identifier_setup = "pindef_confocal";
    #endif
    #ifdef freedcam
        #include "pindef_WEMOS_d1_r32.h"

        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 0;
        pindef.analog_PIN_2 = 0;
        pindef.analog_PIN_3 = 0;

        // Laser PWM pins for CNC Shield
        pindef.LASER_PIN_1 = SPINDLE_ENABLE_PIN; // was SPINDLEPWMPIN; // Spin Dir
        pindef.LASER_PIN_2 = 0; // was SPINDLE_ENABLE_PIN;//  Spin En
        pindef.LASER_PIN_3 = 0;// 

        // Stepper Motor pins
        pindef.STEP_A = 0;
        pindef.STEP_X = 19;
        pindef.STEP_Y = 0;
        pindef.STEP_Z = 0;
        pindef.DIR_A = 0;
        pindef.DIR_X = 21;
        pindef.DIR_Y = 0;
        pindef.DIR_Z = 0;
        pindef.ENABLE = 18;

        // GALVos are always connected to 25/26 
        pindef.dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
        pindef.dac_fake_2 = 0; // Coolant

        // ledarray
        pindef.LED_ARRAY_PIN = 27; //FEED_HOLD_PIN; //27//CYCLE_START_PIN; // was FEED_HOLD_PIN; // FEED HOLD 

        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 0; //  Cycle Start/Resume
        pindef.digital_PIN_2 = 0; //not used/reserved
        pindef.digital_PIN_3 = 0; //not used/reserved

        pindef.identifier_setup = "pindef_freedcam";
    #endif
    #ifdef incubator_microscope_zonly_matrix
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 0;
        pindef.analog_PIN_2 = 0;
        pindef.analog_PIN_3 = 0;
        // Stepper Motor pins
        pindef.STEP_A = 0;
        pindef.STEP_X = 26;
        pindef.STEP_Y = 25;
        pindef.STEP_Z = 17;
        pindef.DIR_A = 0;
        pindef.DIR_X = 16;
        pindef.DIR_Y = 27;
        pindef.DIR_Z = 14;
        pindef.ENABLE = 12;


        // GALVos are always connected to 25/26 
        pindef.dac_fake_1 = 2; // RESET-ABORT just toggles between 1 and 0
        pindef.dac_fake_2 = 34; // Coolant

        // ledarray
        pindef.LED_ARRAY_PIN = 4; // FEED HOLD 

        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 35; //  Cycle Start/Resume
        pindef.digital_PIN_2 = 35; //not used/reserved
        pindef.digital_PIN_3 = 39; //not used/reserved

        pindef.identifier_setup = "pindef_lightsheet_tomo_galvo_espwemos";
    #endif
    #ifdef light_sheet_arduino
        pindef.analog_PIN_1 = 21;
        pindef.analog_PIN_2 = 22;
        pindef.analog_PIN_3 = 0;

        // Stepper Motor pins
        pindef.STEP_X = 2;
        pindef.STEP_Y = 3;
        pindef.STEP_Z = 4;
        pindef.DIR_X = 5;
        pindef.DIR_Y = 6;
        pindef.DIR_Z = 7;
        pindef.ENABLE = 8;

        // Laser PWM pins for CNC Shield
        pindef.LASER_PIN_1 = 9; // X-endstop
        pindef.LASER_PIN_2 = 10;// Y-endstop
        pindef.LASER_PIN_3 = 11;// Z-endstop

        pindef.identifier_setup = "pindef_lightsheet_arduino";
    #endif
    #ifdef light_sheet_esp_tomo
        #define IS_DIGITAL
        //#define IS_DAC
        //#define IS_WIFI
        // ESP32-only
        //#define IS_ANALOG// ESP32-only
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 0;
        pindef.analog_PIN_2 = 0;
        pindef.analog_PIN_3 = 0;
        // Stepper Motor pins
        pindef.STEP_A = 0;
        pindef.STEP_X = 18;
        pindef.STEP_Y = 19;
        pindef.STEP_Z = 23;
        pindef.DIR_A = 0; // on wemos mini 35 does not work
        pindef.DIR_X = 2; // on wemos mini 35 does not work
        pindef.DIR_Y = 33;
        pindef.DIR_Z = 4; // on wemos mini 34 does not work
        pindef.ENABLE = 5;
        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 22; 
        pindef.digital_PIN_2 = 21; 
        pindef.digital_PIN_3 = 17; 
        // Laser PWM pins for CNC Shield
        pindef.LASER_PIN_1 = 0; // X-endstop
        pindef.LASER_PIN_2 = 0;// Y-endstop
        pindef.LASER_PIN_3 = 0;// Z-endstop
        pindef.identifier_setup = "pindef_lightsheet_esp_tomo";
    #endif
    #ifdef lightsheet_esp_wemos
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 0;
        pindef.analog_PIN_2 = 0;
        pindef.analog_PIN_3 = 0;
        // Laser PWM pins for CNC Shield
        pindef.LASER_PIN_1 = SPINDLEPWMPIN; // Spin Dir
        pindef.LASER_PIN_2 = SPINDLE_ENABLE_PIN;//  Spin En
        pindef.LASER_PIN_3 = 0;// 
        // Stepper Motor pins
        pindef.STEP_A = 0;
        pindef.STEP_X = X_STEP_PIN;
        pindef.STEP_Y = Y_STEP_PIN;
        pindef.STEP_Z = Z_STEP_PIN;
        pindef.DIR_A = 0;
        pindef.DIR_X = X_DIRECTION_PIN;
        pindef.DIR_Y = Y_DIRECTION_PIN;
        pindef.DIR_Z = Z_DIRECTION_PIN;
        pindef.ENABLE = STEPPERS_ENABLE_PIN;
        // GALVos are always connected to 25/26 
        pindef.dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
        pindef.dac_fake_2 = 0; // Coolant
        // ledarray
        pindef.LED_ARRAY_PIN = FEED_HOLD_PIN; // FEED HOLD 
        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 0; //  Cycle Start/Resume
        pindef.digital_PIN_2 = 0; //not used/reserved
        pindef.digital_PIN_3 = 0; //not used/reserved
        pindef.identifier_setup = "pindef_lightsheet_espwemos";
    #endif
    #ifdef lightsheet_tomo_galvo_espwemos
        #define IS_DIGITAL
        //#define IS_DAC
        #define IS_DAC_FAKE
        //#define IS_WIFI
        // ESP32-only
        //#define IS_ANALOG// ESP32-only
        #define IS_READSENSOR
        #define IS_PID
        // for reading analog input values
        pindef.ADC_pin_0 = 34;
        pindef.ADC_pin_1 = 0;
        pindef.ADC_pin_2 = 0;
        pindef.N_sensor_avg = 100;
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 0;
        pindef.analog_PIN_2 = 0;
        pindef.analog_PIN_3 = 0;
        // Stepper Motor pins
        pindef.STEP_A = 0;
        pindef.STEP_X = 26;
        pindef.STEP_Y = 25;
        pindef.STEP_Z = 17;
        pindef.DIR_A = 0;
        pindef.DIR_X = 16;
        pindef.DIR_Y = 27;
        pindef.DIR_Z = 14;
        pindef.ENABLE = 12;
        // GALVos are always connected to 25/26 
        pindef.dac_fake_1 = 2; // RESET-ABORT just toggles between 1 and 0
        pindef.dac_fake_2 = 34; // Coolant
        // ledarray
        pindef.LED_ARRAY_PIN = 4; // FEED HOLD 
        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 35; //  Cycle Start/Resume
        pindef.digital_PIN_2 = 35; //not used/reserved
        pindef.digital_PIN_3 = 39; //not used/reserved
        pindef.identifier_setup = "pindef_lightsheet_tomo_galvo_espwemos";
    #endif
    #ifdef lightsheet_tomo_galvo
        #define IS_DIGITAL
        #define IS_DAC
        //#define IS_WIFI
        // ESP32-only
        //#define IS_ANALOG// ESP32-only
        // GALVos are always connected to 25/26 
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 0;
        pindef.analog_PIN_2 = 0;
        pindef.analog_PIN_3 = 0;
        // Stepper Motor pins
        pindef.STEP_A = 0;
        pindef.STEP_X = 18;
        pindef.STEP_Y = 19;
        pindef.STEP_Z = 23;
        pindef.DIR_A = 0; // on wemos mini 35 does not work
        pindef.DIR_X = 2; // on wemos mini 35 does not work
        pindef.DIR_Y = 33;
        pindef.DIR_Z = 4; // on wemos mini 34 does not work
        pindef.ENABLE = 5;
        // ledarray
        pindef.LED_ARRAY_PIN = 32; 
        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 22; 
        pindef.digital_PIN_2 = 21; 
        pindef.digital_PIN_3 = 17; 
        // Laser PWM pins for CNC Shield
        pindef.LASER_PIN_1 = 0; // X-endstop
        pindef.LASER_PIN_2 = 0;// Y-endstop
        pindef.LASER_PIN_3 = 0;// Z-endstop
        pindef.identifier_setup = "pindef_lightsheet_esp_tomo_galvo";
    #endif
    #ifdef lightsheet_tomo_PID_espwemos
        #define IS_DIGITAL
        //#define IS_DAC
        #define IS_DAC_FAKE
        //#define IS_WIFI
        // ESP32-only
        //#define IS_ANALOG// ESP32-only
        #define IS_READSENSOR
        #define IS_PID
        // for reading analog input values
        pindef.ADC_pin_0 = 34;
        pindef.ADC_pin_1 = 0;
        pindef.ADC_pin_2 = 0;
        pindef.N_sensor_avg = 100;
        // analog out (e.g. Lenses)
        pindef.analog_PIN_1 = 0;
        pindef.analog_PIN_2 = 0;
        pindef.analog_PIN_3 = 0;
        // Stepper Motor pins
        pindef.STEP_A = 0;
        pindef.STEP_X = 26;
        pindef.STEP_Y = 25;
        pindef.STEP_Z = 17;
        pindef.DIR_A = 0;
        pindef.DIR_X = 16;
        pindef.DIR_Y = 27;
        pindef.DIR_Z = 14;
        pindef.ENABLE = 12;
        // GALVos are always connected to 25/26 
        pindef.dac_fake_1 = 2; // RESET-ABORT just toggles between 1 and 0
        pindef.dac_fake_2 = 34; // Coolant
        // ledarray
        pindef.LED_ARRAY_PIN = 4; // FEED HOLD 
        // digital out (e.g. camera trigger)
        pindef.digital_PIN_1 = 35; //  Cycle Start/Resume
        pindef.digital_PIN_2 = 35; //not used/reserved
        pindef.digital_PIN_3 = 39; //not used/reserved
        pindef.identifier_setup = "pindef_lightsheet_tomo_galvo_espwemos";
    #endif
    #ifdef lightsheet
    #define IS_ANALOG// ESP32-only


#define IS_DAC

// analog out (e.g. Lenses)
pindef.analog_PIN_1 = 0;
pindef.analog_PIN_2 = 0;
pindef.analog_PIN_3 = 0;

// Definition cellSTORM
pindef.STEP_X = 0;
pindef.STEP_Y = 32;
pindef.STEP_Z = 2;
pindef.DIR_X = 23;
pindef.DIR_Y = 23;
pindef.DIR_Z = 23;
pindef.ENABLE = 22;

// Laser PWM pins
pindef.LASER_PIN_1 = 33;
pindef.LASER_PIN_2 = 0;
pindef.LASER_PIN_3 = 0;

pindef.identifier_setup = "pindef_lightsheet";
    #endif
    #ifdef multicolor_borstel
    #define IS_ANALOG// ESP32-only


//#define IS_DAC

// analog out (e.g. Lenses)
pindef.analog_PIN_1 = 32;
pindef.analog_PIN_2 = 24;
pindef.analog_PIN_3 = 0;

// Motor pins - multicolour fluorescence
pindef.STEP_X = 0;
pindef.STEP_Y = 23;
pindef.STEP_Z = 2;
pindef.DIR_X = 0;
pindef.DIR_Y = 22;
pindef.DIR_Z = 4;
pindef.ENABLE = 5;

pindef.LASER_PIN_1 = 18;
pindef.LASER_PIN_2 = 19;
pindef.LASER_PIN_3 = 21;

pindef.identifier_setup = "multicolour";
    #endif
    #ifdef multicolour_fluorescence_wemos_borstel
    #include "pindef_WEMOS_d1_r32.h"
//#define IS_DIGITAL
//#define IS_DAC
//#define IS_DAC_FAKE
//#define IS_WIFI

//#define IS_ANALOG// ESP32-only


// analog out (e.g. Lenses)
pindef.analog_PIN_1 = 0;
pindef.analog_PIN_2 = 0;
pindef.analog_PIN_3 = 0;

// Laser PWM pins for CNC Shield
pindef.LASER_PIN_1 = SPINDLEPWMPIN; // Spin Dir
pindef.LASER_PIN_2 = SPINDLE_ENABLE_PIN;//  Spin En
pindef.LASER_PIN_3 = 0;//


// Stepper Motor pins
pindef.STEP_A = 0;
pindef.STEP_X = X_STEP_PIN;
pindef.STEP_Y = Y_STEP_PIN;
pindef.STEP_Z = Z_STEP_PIN;
pindef.DIR_A = 0;
pindef.DIR_X = X_DIRECTION_PIN;
pindef.DIR_Y = Y_DIRECTION_PIN;
pindef.DIR_Z = Z_DIRECTION_PIN;
pindef.ENABLE = STEPPERS_ENABLE_PIN;

// GALVos are always connected to 25/26
pindef.dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
pindef.dac_fake_2 = 0; // Coolant

// ledarray
pindef.LED_ARRAY_PIN = FEED_HOLD_PIN; // FEED HOLD

// digital out (e.g. camera trigger)
//pindef.digital_PIN_1 = 0; //  Cycle Start/Resume
//pindef.digital_PIN_2 = 0; //not used/reserved
//pindef.digital_PIN_3 = 0; //not used/reserved

pindef.identifier_setup = "pindef_multicolour_fluorescence_wemos_borstel";
    #endif
    #ifdef multicolour_wemos_lena
    #include "pindef_WEMOS_d1_r32.h"
//#define IS_DIGITAL
//#define IS_DAC
//#define IS_DAC_FAKE
//#define IS_WIFI
 // ESP32-only
//#define IS_ANALOG// ESP32-only

// analog out (e.g. Lenses)
pindef.analog_PIN_1 = 0;
pindef.analog_PIN_2 = 0;
pindef.analog_PIN_3 = 0;

// Laser PWM pins for CNC Shield
pindef.LASER_PIN_1 = X_LIMIT_PIN; // was SPINDLEPWMPIN; // Spin Dir
pindef.LASER_PIN_2 = Y_LIMIT_PIN; // was SPINDLE_ENABLE_PIN;//  Spin En
pindef.LASER_PIN_3 = 0;// 

// Stepper Motor pins
pindef.STEP_A = A_STEP_PIN;
pindef.STEP_X = X_STEP_PIN;
pindef.STEP_Y = Y_STEP_PIN;
pindef.STEP_Z = Z_STEP_PIN;
pindef.DIR_A = A_DIRECTION_PIN;
pindef.DIR_X = X_DIRECTION_PIN;
pindef.DIR_Y = Y_DIRECTION_PIN;
pindef.DIR_Z = Z_DIRECTION_PIN;
pindef.ENABLE = STEPPERS_ENABLE_PIN;

// GALVos are always connected to 25/26 
pindef.dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
pindef.dac_fake_2 = 0; // Coolant

// ledarray
pindef.LED_ARRAY_PIN = FEED_HOLD_PIN; //CYCLE_START_PIN; // was FEED_HOLD_PIN; // FEED HOLD 

// digital out (e.g. camera trigger)
pindef.digital_PIN_1 = 0; //  Cycle Start/Resume
pindef.digital_PIN_2 = 0; //not used/reserved
pindef.digital_PIN_3 = 0; //not used/reserved

pindef.identifier_setup = "pindef_multicolour_wemos_lena";
    #endif
    #ifdef multicolour
    #define IS_ANALOG// ESP32-only


//#define IS_DAC


// analog out (e.g. Lenses)
pindef.analog_PIN_1 = 32;
pindef.analog_PIN_2 = 24;
pindef.analog_PIN_3 = 0;

// Motor pins - multicolour fluorescence
pindef.STEP_X = 0;
pindef.STEP_Y = 23;
pindef.STEP_Z = 2;
pindef.DIR_X = 0;
pindef.DIR_Y = 22;
pindef.DIR_Z = 4;
pindef.ENABLE = 5;

pindef.LASER_PIN_1 = 18;
pindef.LASER_PIN_2 = 19;
pindef.LASER_PIN_3 = 21;

pindef.identifier_setup = "multicolour";
    #endif
    #ifdef oct_eda
    #include "pindef_WEMOS_d1_r32.h"

// Laser PWM pins for CNC Shield
pindef.LASER_PIN_1 = 27; // was SPINDLEPWMPIN; // Spin Dir
pindef.LASER_PIN_2 = 0; // was SPINDLE_ENABLE_PIN;//  Spin En
pindef.LASER_PIN_3 = 0;// 

    #endif
    #ifdef ptychography
    // analog out (e.g. Lenses)
pindef.analog_PIN_1 = 21;
pindef.analog_PIN_2 = 22;
pindef.analog_PIN_3 = 0;

// Stepper Motor pins
pindef.STEP_X = 2;
pindef.STEP_Y = 3;
pindef.STEP_Z = 4;
pindef.DIR_X = 5;
pindef.DIR_Y = 6;
pindef.DIR_Z = 7;
pindef.ENABLE = 8;

// Laser PWM pins for CNC Shield
pindef.LASER_PIN_1 = 9; // X-endstop
pindef.LASER_PIN_2 = 10;// Y-endstop
pindef.LASER_PIN_3 = 11;// Z-endstop

pindef.identifier_setup = "pindef_ptychography";
    #endif
    #ifdef slm
    #define IS_SLM


pindef.TFT_RST = 4;
pindef.TFT_DC = 2; //A0
pindef.TFT_CS = 15; //CS
pindef.TFT_MOSI = 23; //SDA
pindef.TFT_CLK = 18; //SCK



pindef.LED_ARRAY_PIN = 22; // FEED HOLD 

String identifier_setup = "pindef_SLM";
    #endif
    #ifdef uc2standalone
    #include "pindef_WEMOS_d1_r32.h"
//#define IS_DIGITAL
#define IS_DAC
//#define IS_DAC_FAKE
#define IS_WIFI
 // ESP32-only
//#define IS_ANALOG// ESP32-only


// analog out (e.g. Lenses)
pindef.analog_PIN_1 = 0;
pindef.analog_PIN_2 = 0;
pindef.analog_PIN_3 = 0;

// Laser PWM pins for CNC Shield
pindef.LASER_PIN_1 = 4; //ATTENTION 35 is input only!!  // was SPINDLEPWMPIN; // Spin Dir
pindef.LASER_PIN_2 = 32; // was SPINDLE_ENABLE_PIN;//  Spin En
pindef.LASER_PIN_3 = 0;// 

pindef.LIM_X = 17;
pindef.LIM_Y = 4;
pindef.LIM_Z = 15;

// Stepper Motor pins
pindef.STEP_A = 0; // ATTENTION I2C SCL: 22;
pindef.STEP_X = 2;
pindef.STEP_Y = 27;
pindef.STEP_Z = 12;
pindef.DIR_A = 0; // ATTENTION I2C SDA 21;
pindef.DIR_X = 33;
pindef.DIR_Y = 16;
pindef.DIR_Z = 14;
pindef.ENABLE = 13;

// GALVos are always connected to 25/26 
pindef.dac_fake_1 = 0; // RESET-ABORT just toggles between 1 and 0
pindef.dac_fake_2 = 0; // Coolant

// ledarray
pindef.LED_ARRAY_PIN = 17; //35 -> ATTENTION! INPUT ONLY!!!! //CYCLE_START_PIN; // was FEED_HOLD_PIN; // FEED HOLD 

// digital out (e.g. camera trigger)
pindef.digital_PIN_1 = 0; //  Cycle Start/Resume
pindef.digital_PIN_2 = 0; //not used/reserved
pindef.digital_PIN_3 = 0; //not used/reserved

pindef.identifier_setup = "pindef_uc2standalone";
    #endif
    #ifdef xyz_stagescan_ps4
    #define IS_DIGITAL
//#define IS_DAC
#define IS_DAC_FAKE
//#define IS_WIFI
 // ESP32-only
//#define IS_ANALOG// ESP32-only

#define IS_READSENSOR
#define IS_PID

// for reading analog input values
pindef.ADC_pin_0 = 34;
pindef.ADC_pin_1 = 0;
pindef.ADC_pin_2 = 0;
pindef.N_sensor_avg = 100;

// analog out (e.g. Lenses)
pindef.analog_PIN_1 = 0;
pindef.analog_PIN_2 = 0;
pindef.analog_PIN_3 = 0;

// Stepper Motor pins
pindef.STEP_A = 0;
pindef.STEP_X = 26;
pindef.STEP_Y = 25;
pindef.STEP_Z = 17;
pindef.DIR_A = 0;
pindef.DIR_X = 16;
pindef.DIR_Y = 27;
pindef.DIR_Z = 14;
pindef.ENABLE = 12;


// GALVos are always connected to 25/26 
pindef.dac_fake_1 = 2; // RESET-ABORT just toggles between 1 and 0
pindef.dac_fake_2 = 34; // Coolant

// ledarray
pindef.LED_ARRAY_PIN = 4; // FEED HOLD 

// digital out (e.g. camera trigger)
pindef.digital_PIN_1 = 35; //  Cycle Start/Resume
pindef.digital_PIN_2 = 35; //not used/reserved
pindef.digital_PIN_3 = 39; //not used/reserved

pindef.identifier_setup = "pindef_lightsheet_tomo_galvo_espwemos";
    #endif
};
