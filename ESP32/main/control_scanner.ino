#ifdef IS_SCANNER
#include "parameters_scanner.h"
//#include <FreeRTOS.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

void controlGalvoTask( void * parameter ) {
  Serial.println("Starting Scanner Thread");

  while (1) {
    // loop forever

    if (isScanRunning) {
      runScanner();
    }
    else {
      vTaskDelay(100);
    }
  }
  vTaskDelete(NULL);
}


void runScanner() {
  if (DEBUG) Serial.println("Start FrameStack");
  int roundTripCounter = 0;

  for (int iFrame = 0; iFrame < scannernFrames; iFrame++) {
    // shifting phase in x
    for (int idelayX = scannerXFrameMin; idelayX < scannerXFrameMax; idelayX++) {
      // shifting phase in y
      for (int idelayY = scannerYFrameMin; idelayY < scannerYFrameMax; idelayY++) {
        // iteratinv over all pixels in x
        for (int ix = scannerxMin; ix < scannerxMax - scannerXStep; ix += scannerXStep) {
          // move X-mirror
          dacWrite(scannerPinX, ix + idelayX);
          //Serial.print("X");Serial.print(ix + idelayX);

          for (int iy = scanneryMin; iy < scanneryMax - scannerYStep; iy += scannerYStep) {
            // move Y-mirror triangle
            int scannerPosY = 0;
            if ((roundTripCounter % 2) == 0 ) {
              scannerPosY = iy + idelayY;
            }
            else {
              scannerPosY = 255 - (iy + idelayY);
            }
            dacWrite(scannerPinY, scannerPosY);
            //Serial.print("Y");Serial.println(scannerPosY);

            // expose Laser
            ledcWrite(PWM_CHANNEL_LASER_1, scannerLaserVal);
            delay(scannerExposure);
            ledcWrite(PWM_CHANNEL_LASER_1, 0);
          }
        }
      }
    }
  }
  if (DEBUG) Serial.println("Ending FrameStack");
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
    scanneryMin = 0;
    scannerxMax = 255;
    scanneryMax = 255;
    scannerExposure = 0;
    scannerEnable = 0;
    scannerLaserVal = 255;
    scannerXFrameMax = 5;
    scannerXFrameMin = 0;
    scannerYFrameMax = 5;
    scannerYFrameMin = 0;
    scannerXStep = 5;
    scannerYStep = 5;
    scannernFrames = 1;

    if (jsonDocument.containsKey("scannernFrames")) {
      scannernFrames = jsonDocument["scannernFrames"];
    }
    if (jsonDocument.containsKey("scannerXFrameMax")) {
      scannerXFrameMax = jsonDocument["scannerXFrameMax"];
    }
    if (jsonDocument.containsKey("scannerXFrameMin")) {
      scannerXFrameMin = jsonDocument["scannerXFrameMin"];
    }
    if (jsonDocument.containsKey("scannerYFrameMax")) {
      scannerYFrameMax = jsonDocument["scannerYFrameMax"];
    }
    if (jsonDocument.containsKey("scannerYFrameMin")) {
      scannerYFrameMin = jsonDocument["scannerYFrameMin"];
    }
    if (jsonDocument.containsKey("scannerXStep")) {
      scannerXStep = jsonDocument["scannerXStep"];
    }
    if (jsonDocument.containsKey("scannerYStep")) {
      scannerYStep = jsonDocument["scannerYStep"];
    }
    if (jsonDocument.containsKey("scannerxMin")) {
      scannerxMin = jsonDocument["scannerxMin"];
    }
    if (jsonDocument.containsKey("scannerLaserVal")) {
      scannerLaserVal = jsonDocument["scannerLaserVal"];
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
    if (jsonDocument.containsKey("scannerExposure")) {
      scannerExposure = jsonDocument["scannerExposure"];
    }
    if (jsonDocument.containsKey("scannerEnable")) {
      scannerEnable = jsonDocument["scannerEnable"];
    }

    jsonDocument.clear();
    Serial.println("Start controlGalvoTask");
    isScanRunning = scannerEnable; // Trigger a frame acquisition
    Serial.println("Done with setting up Tasks");
    jsonDocument["return"] = 1;

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
/******************************************* SCANNER     *******************************************/
/***************************************************************************************************/
/****************************************** ********************************************************/


void setup_scanner() {
  runScanner(); // run not as a task
  disableCore0WDT();
  xTaskCreate(controlGalvoTask, "controlGalvoTask", 10000, NULL, 1, NULL);
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
