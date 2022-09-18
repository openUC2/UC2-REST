#include "SlmController.h"

SlmController::SlmController(/* args */){};
SlmController::~SlmController(){};

// Custom function accessible by the API
void SlmController::act() {

  // here you can do something
  if(DEBUG) Serial.println("slm_act_fct");
  isBusy = true;

  const char* slmMode = (*WifiController::getJDoc())["slmMode"]; // "ring", "clear"

  // individual pattern gets adressed
  // PYTHON: send_LEDMatrix_array(self, led_pattern, timeout=1)
  if(strcmp(slmMode, "circle")) 
  {
    if (DEBUG) Serial.println("circle");
    int posX = 0;
    int posY = 0;
    int radius = 0;
    uint16_t color = 0;

    if(WifiController::getJDoc()->containsKey("posX"))
      posX = (*WifiController::getJDoc())["posX"];
    if(WifiController::getJDoc()->containsKey("posY"))
      posY = (*WifiController::getJDoc())["posY"];
    if(WifiController::getJDoc()->containsKey("radius"))
      radius = (*WifiController::getJDoc())["radius"];
    if(WifiController::getJDoc()->containsKey("color"))
      color = (*WifiController::getJDoc())["color"];
    tft.fillCircle(posX, posY, radius, color);
  }
  // PYTHON: send_LEDMatrix_array(self, led_pattern, timeout=1)
  if(strcmp(slmMode, "rect")) 
  {
    if(DEBUG) Serial.println("rect");
    int posX = 0;
    int posY = 0;
    int nX = 0;
    int nY = 0;
    uint16_t color = 0;

    if(WifiController::getJDoc()->containsKey("posX"))
      posX = (*WifiController::getJDoc())["posX"];
    if(WifiController::getJDoc()->containsKey("posY"))
      posY = (*WifiController::getJDoc())["posY"];
    if(WifiController::getJDoc()->containsKey("nX"))
      nX = (*WifiController::getJDoc())["nX"];
    if(WifiController::getJDoc()->containsKey("nY"))
      nY = (*WifiController::getJDoc())["nY"];
    if(WifiController::getJDoc()->containsKey("color"))
      color = (*WifiController::getJDoc())["color"];
    tft.drawRect(posX, posY, nX, nY, color);
  }

  if (strcmp(slmMode, "full")) {
    if (DEBUG) Serial.println("full");
    uint16_t color = (*WifiController::getJDoc())["color"];
    tft.fillScreen(color);
  }
  if (strcmp(slmMode, "clear")) {
    if (DEBUG) Serial.println("clear");
    tft.fillScreen(ST77XX_BLACK);
  }
  // only if a single led will be updated, all others stay the same
  // PYTHON: send_LEDMatrix_single(self, indexled=0, intensity=(255,255,255), timeout=1)
  else if (strcmp(slmMode, "image")) {
    if (DEBUG) Serial.println("image");
    int startX = (*WifiController::getJDoc())["startX"];
    int startY = (*WifiController::getJDoc())["startY"];
    int endX = (*WifiController::getJDoc())["endX"];
    int endY = (*WifiController::getJDoc())["endY"];

    int nX = endX-startX;
    int nY = endY-startY;
    for (int ix = 0; ix < nX; ix++) {
      for (int iy = 0; iy < nY; iy++) {
        uint16_t color = (*WifiController::getJDoc())["color"][ix*(nX)+iy];  //Implicit cast
        Serial.println(color);
        tft.drawPixel(iy+nY, ix+nX, color);
      }
    }
  }

  WifiController::getJDoc()->clear();
  (*WifiController::getJDoc())["return"] = 1;
  isBusy = false;
}

void SlmController::set() {
  /*
    if (WifiController::getJDoc()["LED_ARRAY_PIN"] != 0) {
    //if (DEBUG) Serial.print("LED_ARRAY_PIN "); Serial.println(WifiController::getJDoc()["LED_ARRAY_PIN"]);
    LED_ARRAY_PIN = WifiController::getJDoc()["LED_ARRAY_PIN"];
    }

    if (WifiController::getJDoc()["LED_N_X"] != 0) {
    //if (DEBUG) Serial.print("LED_N_X "); Serial.println(WifiController::getJDoc()["LED_N_X"]);
    LED_N_X = WifiController::getJDoc()["LED_N_X"];
    }

    if (WifiController::getJDoc()["LED_N_Y"] != 0) {
    //if (DEBUG) Serial.print("LED_N_Y "); Serial.println(WifiController::getJDoc()["LED_N_Y"]);
    LED_N_Y = WifiController::getJDoc()["LED_N_Y"];
    }
  */
  (*WifiController::getJDoc()).clear();
  (*WifiController::getJDoc())["return"] = 1;
}



// Custom function accessible by the API
void SlmController::get() {
  /*
    WifiController::getJDoc().clear();
    WifiController::getJDoc()["LED_ARRAY_PIN"] = LED_ARRAY_PIN;
    WifiController::getJDoc()["LED_N_X"] = LED_N_X;
    WifiController::getJDoc()["LED_N_Y"] = LED_N_Y;
  */
}



/***************************************************************************************************/
/*******************************************  LED Array  *******************************************/
/***************************************************************************************************/
/*******************************FROM OCTOPI ********************************************************/

void SlmController::setup() {
  if(DEBUG) Serial.println("Initializing SLM");
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.fillScreen(ST77XX_WHITE);
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
  if(DEBUG) Serial.println("Done Initializing SLM");
}

/*
   Auxilary functions
*/

// https://github.com/fatur04/arduino/blob/master/ESP32_WEBSOCKET_SERVER_WITH_TFT_JPEG_V1/ESP32_WEBSOCKET_SERVER_WITH_TFT_JPEG_V1.ino

//====================================================================================
//   Opens the image file and prime the Jpeg decoder
//====================================================================================
void SlmController::drawJpeg(String filename, int xpos, int ypos) {

  Serial.println("===========================");
  Serial.print("Drawing file: "); Serial.println(filename);
  Serial.println("===========================");

  // Open the named file (the Jpeg decoder library will close it after rendering image)
  File jpegFile = SPIFFS.open( filename, "r");    // File handle reference for SPIFFS
  //  File jpegFile = SD.open( filename, FILE_READ);  // or, file handle reference for SD library

  if ( !jpegFile ) {
    Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
    return;
  }

  // Use one of the three following methods to initialise the decoder:
  //boolean decoded = JpegDec.decodeFsFile(jpegFile); // Pass a SPIFFS file handle to the decoder,
  //boolean decoded = JpegDec.decodeSdFile(jpegFile); // or pass the SD file handle to the decoder,
  boolean decoded = JpegDec.decodeFsFile(filename);  // or pass the filename (leading / distinguishes SPIFFS files)
  // Note: the filename can be a String or character array type
  if (decoded) {
    // print information about the image to the serial port
    jpegInfo();

    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  }
  else {
    Serial.println("Jpeg file format not supported!");
  }
}

//====================================================================================
//   Decode and render the Jpeg image onto the TFT screen
//====================================================================================
void SlmController::jpegRender(int xpos, int ypos) {

  // retrieve infomration about the image
  uint16_t  *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while ( JpegDec.read()) {

    // save a pointer to the image block
    pImg = JpegDec.pImage;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      for (int h = 1; h < win_h - 1; h++)
      {
        memcpy(pImg + h * win_w, pImg + (h + 1) * mcu_w, win_w << 1);
      }
    }

    // draw image MCU block only if it will fit on the screen
    if ( ( mcu_x + win_w) <= tft.width() && ( mcu_y + win_h) <= tft.height())
    {
      tft.drawRGBBitmap(mcu_x, mcu_y, pImg, win_w, win_h);
    }

    else if ( ( mcu_y + win_h) >= tft.height()) JpegDec.abort();

  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime; // Calculate the time it took

  // print the results to the serial port
  Serial.print  ("Total render time was    : "); Serial.print(drawTime); Serial.println(" ms");
  Serial.println("=====================================");

}

//====================================================================================
//   Print information decoded from the Jpeg image
//====================================================================================
void SlmController::jpegInfo() {

  Serial.println("===============");
  Serial.println("JPEG image info");
  Serial.println("===============");
  Serial.print  ("Width      :"); Serial.println(JpegDec.width);
  Serial.print  ("Height     :"); Serial.println(JpegDec.height);
  Serial.print  ("Components :"); Serial.println(JpegDec.comps);
  Serial.print  ("MCU / row  :"); Serial.println(JpegDec.MCUSPerRow);
  Serial.print  ("MCU / col  :"); Serial.println(JpegDec.MCUSPerCol);
  Serial.print  ("Scan type  :"); Serial.println(JpegDec.scanType);
  Serial.print  ("MCU width  :"); Serial.println(JpegDec.MCUWidth);
  Serial.print  ("MCU height :"); Serial.println(JpegDec.MCUHeight);
  Serial.println("===============");
  Serial.println("");
}

//====================================================================================
//   Open a Jpeg file and send it to the Serial port in a C array compatible format
//====================================================================================
void SlmController::createArray(const char *filename) {

  // Open the named file
  fs::File jpgFile = SPIFFS.open( filename, "r");    // File handle reference for SPIFFS
  //  File jpgFile = SD.open( filename, FILE_READ);  // or, file handle reference for SD library

  if ( !jpgFile ) {
    Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
    return;
  }

  uint8_t data;
  byte line_len = 0;
  Serial.println("");
  Serial.println("// Generated by a JPEGDecoder library example sketch:");
  Serial.println("// https://github.com/Bodmer/JPEGDecoder");
  Serial.println("");
  Serial.println("#if defined(__AVR__)");
  Serial.println("  #include <avr/pgmspace.h>");
  Serial.println("#endif");
  Serial.println("");
  Serial.print  ("const uint8_t ");
  while (*filename != '.') Serial.print(*filename++);
  Serial.println("[] PROGMEM = {"); // PROGMEM added for AVR processors, it is ignored by Due

  while ( jpgFile.available()) {

    data = jpgFile.read();
    Serial.print("0x"); if (abs(data) < 16) Serial.print("0");
    Serial.print(data, HEX); Serial.print(",");// Add value and comma
    line_len++;
    if ( line_len >= 32) {
      line_len = 0;
      Serial.println();
    }

  }

  Serial.println("};\r\n");
  jpgFile.close();
}
