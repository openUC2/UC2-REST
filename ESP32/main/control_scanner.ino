#ifdef IS_SCANNER
#include "parameters_scanner.h"

void controlGalvoTask( void * parameter ) {

  scannerPinX = 25;
  scannerPinY = 26;
  scannerPinLaser = 4;

  scannerxMin = 0;
  scannerXOff = 5;
  scanneryMin = 0;
  scannerYOff = 5;
  scannerxMax = 255;
  scanneryMax = 255;
  scannertDelay = 0;
  scannerEnable = 0;
  scannerExposure = 5;

  int roundTripCounter = 0;
  int LASERval = 20000;

  for (int idelayX = 0; idelayX < scannerXOff; idelayX++) {
    for (int idelayY = 0; idelayY < scannerYOff; idelayY++) {
      for (int ix = scannerxMin; ix < scannerxMax - scannerXOff; ix += scannerXOff) {
        // move X-mirror
        ledcWrite(scannerPinX, ix + idelayX);

        for (int iy = scanneryMin; iy < scanneryMax - scannerYOff; iy += scannerYOff) {
          // move Y-mirror triangle
          if (roundTripCounter % 2)
            ledcWrite(scannerPinX, ix + idelayX);
          else
            ledcWrite(scannerPinX, 255 - (ix + idelayX));


          // expose Laser
          ledcWrite(PWM_CHANNEL_LASER_1, LASERval);
          delay(scannerExposure);
          ledcWrite(PWM_CHANNEL_LASER_1, 0);
        }
      }
    }
    vTaskDelete(NULL);
  }
}




// Custom function accessible by the API
void scanner_act_fct() {

  // here you can do something
  if (DEBUG) Serial.println("scanner_act_fct");

  const char* scannerMode = jsonDocument["scannerMode"]; // "classic"
  if (strcmp(scannerMode, "classic") == 0) {
    if (DEBUG) Serial.println("classic");

    // assert values
    scannerxMin = 0;
    scannerXOff = 5;
    scanneryMin = 0;
    scannerYOff = 5;
    scannerxMax = 255;
    scanneryMax = 255;
    scannertDelay = 0;
    scannerEnable = 0;

    if (jsonDocument.containsKey("scannerxMin")) {
      scannerxMin = jsonDocument["scannerxMin"];
    }
    if (jsonDocument.containsKey("scanneryMin")) {
      scanneryMin = jsonDocument["scanneryMin"];
    }
    if (jsonDocument.containsKey("scannerxMax")) {
      scannerxMax = jsonDocument["scannerxMax"];
    }
    if (jsonDocument.containsKey("scanneryMax")) {
      scanneryMax = jsonDocument["scanneryMax"];
    }
    if (jsonDocument.containsKey("scannertDelay")) {
      scannertDelay = jsonDocument["scannertDelay"];
    }
    if (jsonDocument.containsKey("scannerEnable")) {
      scannerEnable = jsonDocument["scannerEnable"];
    }
    if (jsonDocument.containsKey("scannerXOff")) {
      scannerXOff = jsonDocument["scannerXOff"];
    }
    if (jsonDocument.containsKey("scannerYOff")) {
      scannerYOff = jsonDocument["scannerYOff"];
    }

    Serial.println("Start controlGalvoTask");
    xTaskCreatePinnedToCore(controlGalvoTask, "controlGalvoTask", 10000, NULL, 1, NULL, 1);
    Serial.println("Done with setting up Tasks");


    jsonDocument.clear();
    jsonDocument["return"] = 1;
    isBusy = false;
  }
}

void scanner_set_fct() {
  jsonDocument.clear();
  jsonDocument["return"] = 1;
}



// Custom function accessible by the API
void scanner_get_fct() {
  jsonDocument.clear();
  jsonDocument["return"] = 0;
}



/***************************************************************************************************/
/*******************************************  LED Array  *******************************************/
/***************************************************************************************************/
/*******************************FROM OCTOPI ********************************************************/


void setup_scanner() {
}

/*
   wrapper for HTTP requests
*/


#ifdef IS_WIFI
void scanner_act_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  scanner_act_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void scanner_get_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  scanner_get_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}

// wrapper for HTTP requests
void scanner_set_fct_http() {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  scanner_set_fct();
  serializeJson(jsonDocument, output);
  server.send(200, "application/json", output);
}
#endif
#endif
