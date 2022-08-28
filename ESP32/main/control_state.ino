#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include"esp_gap_bt_api.h"
#include "esp_err.h"

static inline int8_t sgn(int val) {
  if (val < 0) return -1;
  if (val == 0) return 0;
  return 1;
}

// Custom function accessible by the API
void state_act_fct() {
  // here you can do something
  if (DEBUG) Serial.println("state_act_fct");

  // assign default values to thhe variables
  if (jsonDocument.containsKey("restart")) {
    ESP.restart();
  }
  // assign default values to thhe variables
  if (jsonDocument.containsKey("delay")) {
    int mdelayms = jsonDocument["delay"];
    delay(mdelayms);
  }
  if (jsonDocument.containsKey("isBusy")) {
    isBusy = jsonDocument["isBusy"];
  }

  if (jsonDocument.containsKey("pscontroller")) {
    IS_PSCONTROLER_ACTIVE = jsonDocument["pscontroller"];
  }
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}

void state_set_fct() {
  // here you can set parameters

  int isdebug = jsonDocument["isdebug"];
  DEBUG = isdebug;
  jsonDocument.clear();
  jsonDocument["return"] = 1;

}

// Custom function accessible by the API
void state_get_fct() {
  // GET SOME PARAMETERS HERE
  if (jsonDocument.containsKey("isBusy")) {
    jsonDocument.clear();
    jsonDocument["isBusy"] = isBusy; // returns state of function that takes longer to finalize (e.g. motor)
  }

  else if (jsonDocument.containsKey("pscontroller")) {
    jsonDocument.clear();
    jsonDocument["pscontroller"] = IS_PSCONTROLER_ACTIVE; // returns state of function that takes longer to finalize (e.g. motor)
  }
  else {
    jsonDocument.clear();
    jsonDocument["identifier_name"] = identifier_name;
    jsonDocument["identifier_id"] = identifier_id;
    jsonDocument["identifier_date"] = identifier_date;
    jsonDocument["identifier_author"] = identifier_author;
    jsonDocument["identifier_setup"] = identifier_setup;
  }
}

void printInfo() {
  if (DEBUG) Serial.println("You can use this software by sending JSON strings, A full documentation can be found here:");
  if (DEBUG) Serial.println("https://github.com/openUC2/UC2-REST/");
  //Serial.println("A first try can be: \{\"task\": \"/state_get\"");
}




// Check if Bluetoothdevice exists?
//https://github.com/espressif/arduino-esp32/blob/master/libraries/BluetoothSerial/examples/bt_remove_paired_devices/bt_remove_paired_devices.ino
//https://github.com/aed3/PS4-esp32/issues/40
#define PAIR_MAX_DEVICES 20
uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
char bda_str[18];
#define REMOVE_BONDED_DEVICES 1   // <- Set to 0 to view all bonded devices addresses, set to 1 to remove

char *bda2str(const uint8_t* bda, char *str, size_t size)
{
  if (bda == NULL || str == NULL || size < 18) {
    return NULL;
  }
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
          bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
  return str;
}
void clearBlueetoothDevice() {
  Serial.print("ESP32 bluetooth address: "); Serial.println(bda2str(esp_bt_dev_get_address(), bda_str, 18));
  // Get the numbers of bonded/paired devices in the BT module
  int count = esp_bt_gap_get_bond_device_num();
  if (!count) {
    Serial.println("No bonded device found.");
  } else {
    Serial.print("Bonded device count: "); Serial.println(count);
    if (PAIR_MAX_DEVICES < count) {
      count = PAIR_MAX_DEVICES;
      Serial.print("Reset bonded device count: "); Serial.println(count);
    }
    esp_err_t tError =  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
    if (ESP_OK == tError) {
      for (int i = 0; i < count; i++) {
        Serial.print("Found bonded device # "); Serial.print(i); Serial.print(" -> ");
        Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));
        if (REMOVE_BONDED_DEVICES) {
          esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
          if (ESP_OK == tError) {
            Serial.print("Removed bonded device # ");
          } else {
            Serial.print("Failed to remove bonded device # ");
          }
          Serial.println(i);
        }
      }
    }
  }
}



/*
   wrapper for HTTP requests
*/

#ifdef IS_WIFI
void state_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  state_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void state_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  state_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void state_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  state_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
