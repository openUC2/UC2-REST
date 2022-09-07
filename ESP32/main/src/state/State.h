#ifndef State_h
#define State_h

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include"esp_gap_bt_api.h"
#include "esp_err.h"

#include <ArduinoJson.h>
#include "../../pinstruct.h"

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

    public:
    State();
    ~State();
    bool DEBUG = false;
    DynamicJsonDocument * jsonDocument;
    PINDEF * pins;

    const char*  identifier_name = "UC2_Feather";
    const char*  identifier_id = "V0.1";
    const char*  identifier_date = "2022-02-04";
    const char*  identifier_author = "BD";


    // timing variables
    unsigned long startMillis;
    unsigned long currentMillis;
    bool isBusy = false;

    void act();
    void set();
    void get();
    void printInfo();
    void clearBlueetoothDevice();
};

static State * state;

#endif