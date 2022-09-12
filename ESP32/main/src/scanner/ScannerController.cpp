
#include "../../config.h"
#ifdef IS_SCANNER
#include "ScannerController.h"


ScannerController::ScannerController()
{
    
};
ScannerController::~ScannerController()
{
    
};


void ScannerController::background() {
  if (DEBUG) Serial.println("Start FrameStack");
  int roundTripCounter = 0;
  /*
    {"task": "/scanner_act",
    "scannernFrames":100,
    "scannerMode":"classic",
    "scannerXFrameMin":0,
    "scannerXFrameMax":255,
    "scannerYFrameMin":0,
    "scannerYFrameMax":255,
    "scannerEnable":0,
    "scannerXFrameMin":1,
    "scannerXFrameMax":1,
    "scannerYFrameMin":1,
    "scannerYFrameMax":1,
    "scannerXStep":15,
    "scannerYStep":15,
    "scannerLaserVal":32000,
    "scannerExposure":10,
    "scannerDelay":1000}
  */

  for (int iFrame = 0; iFrame <= scannernFrames; iFrame++) {
    // shifting phase in x
    for (int idelayX = scannerXFrameMin; idelayX <= scannerXFrameMax; idelayX++) {
      // shifting phase in y
      for (int idelayY = scannerYFrameMin; idelayY <= scannerYFrameMax; idelayY++) {
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
              scannerPosY = (scanneryMax - scannerYStep) - (iy + idelayY);
            }
            roundTripCounter++;
            dacWrite(scannerPinY, scannerPosY);
            //Serial.print("Y");Serial.println(scannerPosY);
            delayMicroseconds(scannerDelay);
            // expose Laser
            #ifdef IS_LASER
              ledcWrite(laser.PWM_CHANNEL_LASER_1, scannerLaserVal); //digitalWrite(LASER_PIN_1, HIGH); //
              delayMicroseconds(scannerExposure);
              ledcWrite(laser.PWM_CHANNEL_LASER_1, 0); //             digitalWrite(LASER_PIN_1, LOW); //
              delayMicroseconds(scannerDelay);
            #endif
          }
        }
      }
    }
  }
  scannernFrames = 0;
  if (DEBUG) Serial.println("Ending FrameStack");
}



// Custom function accessible by the API
void ScannerController::act(DynamicJsonDocument * jsonDocument) {

  // here you can do something
  if (DEBUG) Serial.println("scanner_act_fct");

  // select scanning mode
  const char* scannerMode = (*jsonDocument)["scannerMode"];


  if (strcmp(scannerMode, "pattern") == 0) {
    if (DEBUG) Serial.println("pattern");
    // individual pattern gets adressed
    int arraySize = 0;
    scannerExposure = 0;
    scannerLaserVal = 32000;
    scannernFrames = 1;
    scannerDelay = 0;

    if (jsonDocument->containsKey("scannerExposure")) {
      scannerExposure = (*jsonDocument)["scannerExposure"];
    }
    if (jsonDocument->containsKey("scannerLaserVal")) {
      scannerLaserVal = (*jsonDocument)["scannerLaserVal"];
    }
    if (jsonDocument->containsKey("scannerDelay")) {
      scannerDelay = (*jsonDocument)["scannerDelay"];
    }
    if (jsonDocument->containsKey("scannernFrames")) {
      scannernFrames = (*jsonDocument)["scannernFrames"];
    }
    if (jsonDocument->containsKey("arraySize")) {
      arraySize = (*jsonDocument)["arraySize"];
    }

    for (int iFrame = 0; iFrame < scannernFrames; iFrame++) {
      for (int i = 0; i < arraySize; i++) { //Iterate through results
        int scannerIndex = (*jsonDocument)["i"][i];  //Implicit cast
        int scannerPosY = scannerIndex % 255;
        int scannerPosX = scannerIndex / 255;
        dacWrite(scannerPinY, scannerPosY);
        dacWrite(scannerPinX, scannerPosX);

        /*
         * Serial.print(scannerPosY);
        Serial.print("/");
        Serial.println(scannerPosX);
         */
         
        //Serial.print("Y");Serial.println(scannerPosY);
        delayMicroseconds(scannerDelay);
        // expose Laser
        #ifdef IS_LASER
          ledcWrite(laser.PWM_CHANNEL_LASER_1, scannerLaserVal); //digitalWrite(LASER_PIN_1, HIGH); //
          delayMicroseconds(scannerExposure);
          ledcWrite(laser.PWM_CHANNEL_LASER_1, 0); //             digitalWrite(LASER_PIN_1, LOW); //
          delayMicroseconds(scannerDelay);
        #endif
      }
    }
  }

  else if (strcmp(scannerMode, "classic") == 0) {
    if (DEBUG) Serial.println("classic");

    // assert values
    scannerxMin = 0;
    scanneryMin = 0;
    scannerxMax = 255;
    scanneryMax = 255;
    scannerExposure = 0;
    scannerEnable = 0;
    scannerLaserVal = 32000;

    scannerXFrameMax = 5;
    scannerXFrameMin = 0;
    scannerYFrameMax = 5;
    scannerYFrameMin = 0;
    scannerXStep = 5;
    scannerYStep = 5;
    scannernFrames = 1;

    scannerDelay = 0;



    if (jsonDocument->containsKey("scannernFrames")) {
      scannernFrames = (*jsonDocument)["scannernFrames"];
    }
    if (jsonDocument->containsKey("scannerXFrameMax")) {
      scannerXFrameMax = (*jsonDocument)["scannerXFrameMax"];
    }
    if (jsonDocument->containsKey("scannerXFrameMin")) {
      scannerXFrameMin = (*jsonDocument)["scannerXFrameMin"];
    }
    if (jsonDocument->containsKey("scannerYFrameMax")) {
      scannerYFrameMax = (*jsonDocument)["scannerYFrameMax"];
    }
    if (jsonDocument->containsKey("scannerYFrameMin")) {
      scannerYFrameMin = (*jsonDocument)["scannerYFrameMin"];
    }
    if (jsonDocument->containsKey("scannerXStep")) {
      scannerXStep = (*jsonDocument)["scannerXStep"];
    }
    if (jsonDocument->containsKey("scannerYStep")) {
      scannerYStep = (*jsonDocument)["scannerYStep"];
    }
    if (jsonDocument->containsKey("scannerxMin")) {
      scannerxMin = (*jsonDocument)["scannerxMin"];
    }
    if (jsonDocument->containsKey("scannerLaserVal")) {
      scannerLaserVal = (*jsonDocument)["scannerLaserVal"];
    }
    if (jsonDocument->containsKey("scanneryMin")) {
      scanneryMin = (*jsonDocument)["scanneryMin"];
    }
    if (jsonDocument->containsKey("scannerxMax")) {
      scannerxMax = (*jsonDocument)["scannerxMax"];
    }
    if (jsonDocument->containsKey("scanneryMax")) {
      scanneryMax = (*jsonDocument)["scanneryMax"];
    }
    if (jsonDocument->containsKey("scannerExposure")) {
      scannerExposure = (*jsonDocument)["scannerExposure"];
    }
    if (jsonDocument->containsKey("scannerEnable")) {
      scannerEnable = (*jsonDocument)["scannerEnable"];
    }
    if (jsonDocument->containsKey("scannerDelay")) {
      scannerDelay = (*jsonDocument)["scannerDelay"];
    }

    if (DEBUG) Serial.print("scannerxMin "); Serial.println(scannerxMin);
    if (DEBUG) Serial.print("scanneryMin "); Serial.println(scanneryMin );
    if (DEBUG) Serial.print("scannerxMax "); Serial.println(scannerxMax );
    if (DEBUG) Serial.print("scanneryMax "); Serial.println(scanneryMax );
    if (DEBUG) Serial.print("scannerExposure "); Serial.println(scannerExposure );
    if (DEBUG) Serial.print("scannerEnable "); Serial.println(scannerEnable);
    if (DEBUG) Serial.print("scannerLaserVal "); Serial.println(scannerLaserVal );
    if (DEBUG) Serial.print("scannerXFrameMax "); Serial.println(scannerXFrameMax);
    if (DEBUG) Serial.print("scannerXFrameMin "); Serial.println(scannerXFrameMin);
    if (DEBUG) Serial.print("scannerYFrameMax "); Serial.println(scannerYFrameMax);
    if (DEBUG) Serial.print("scannerYFrameMin "); Serial.println(scannerYFrameMin);
    if (DEBUG) Serial.print("scannerXStep "); Serial.println(scannerXStep );
    if (DEBUG) Serial.print("scannerYStep "); Serial.println(scannerYStep);
    if (DEBUG) Serial.print("scannernFrames "); Serial.println(scannernFrames);
    if (DEBUG) Serial.print("scannerDelay "); Serial.println(scannerDelay);

    jsonDocument->clear();
    if (DEBUG) Serial.println("Start controlGalvoTask");
    isScanRunning = scannerEnable; // Trigger a frame acquisition
    if (DEBUG) Serial.println("Done with setting up Tasks");
    (*jsonDocument)["return"] = 1;

  }
}

void ScannerController::set(DynamicJsonDocument * jsonDocument) {
  jsonDocument->clear();
  (*jsonDocument)["return"] = 1;
}



// Custom function accessible by the API
void ScannerController::get(DynamicJsonDocument * jsonDocument) {
  jsonDocument->clear();
  (*jsonDocument)["return"] = 0;
}



/***************************************************************************************************/
/******************************************* SCANNER     *******************************************/
/***************************************************************************************************/
/****************************************** ********************************************************/

void ScannerController::setup(PINDEF * pins) {
  this->pins = pins;
  background(); // run not as a task
  disableCore0WDT();
  xTaskCreate(ScannerController::controlGalvoTask, "controlGalvoTask", 10000, this, 1, NULL);
}

void ScannerController::controlGalvoTask( void * parameter ) {
    ScannerController * sc = (ScannerController*)parameter;
    Serial.println("Starting Scanner Thread");
    while (1) {
        // loop forever
        if (sc->isScanRunning || sc->scannernFrames > 0) {
            sc->background();
        }
        else {
        vTaskDelay(100);
        }
    }
    vTaskDelete(NULL);
}

#endif

