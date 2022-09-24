#pragma once
#include "../../config.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include"esp_gap_bt_api.h"
#include "esp_err.h"

#include <ArduinoJson.h>
#include "../../pinstruct.h"
#include "../config/JsonKeys.h"
#include "esp_log.h"
#if defined IS_PS3 || defined IS_PS4
#include "../gamepads/ps_3_4_controller.h"
#endif

static int8_t sgn(int val) {
  if (val < 0) return -1;
  if (val == 0) return 0;
  return 1;
}

class State
{

    private:
    char *bda2str(const uint8_t* bda, char *str, size_t size);
    // Check if Bluetoothdevice exists?
    //https://github.com/espressif/arduino-esp32/blob/master/libraries/BluetoothSerial/examples/bt_remove_paired_devices/bt_remove_paired_devices.ino
    //https://github.com/aed3/PS4-esp32/issues/40
    #define PAIR_MAX_DEVICES 20
    uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
    char bda_str[18];
    #define REMOVE_BONDED_DEVICES 1   // <- Set to 0 to view all bonded devices addresses, set to 1 to remove
    const char* TAG = "State";
    public:
    State();
    ~State();
    bool DEBUG = false;
    PINDEF * pins;

    const char*  identifier_name = "UC2_Feather";
    const char*  identifier_id = "V1.2";
    const char*  identifier_date = __DATE__ "" __TIME__;
    const char*  identifier_author = "BD";
    const char* IDENTIFIER_NAME = "";


    // timing variables
    unsigned long startMillis;
    unsigned long currentMillis;
    bool isBusy = false;

    void act();
    void set();
    void get();
    void setup(PINDEF * pins);
    void printInfo();
    void clearBlueetoothDevice();

    void getDefaultPinDef(PINDEF * pindef);
};

static State state;