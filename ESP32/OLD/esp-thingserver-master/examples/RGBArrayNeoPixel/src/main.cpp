// -*- mode: c++;  c-basic-offset: 2 -*-
/**
 * Simple server compliant with Mozilla's proposed WoT API
 * Originally based on the HelloServer example
 * Tested on ESP8266, ESP32, Arduino boards with WINC1500 modules (shields or
 * MKR1000)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Arduino.h>
#include <NeoPixelBus.h>

// Make sure we use an async web server for the WiFi manager
#define WM_ASYNC
#include <AsyncWiFiManager.h>

#include <Thing.h>
#include <WebThingAdapter.h>

#define PIN 32
#define NUM_LEDS 32

// Create NeoPixel strip object
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUM_LEDS, PIN);

WebThingAdapter *adapter;

const char *deviceTypes[] = {"Light", "OnOffSwitch", "ColorControl", nullptr};
ThingDevice device("dimmable-color-light", "Dimmable Color Light",
                   deviceTypes);

ThingProperty deviceOn("on", "Whether the led is turned on", BOOLEAN,
                       "OnOffProperty");
ThingProperty deviceLevel("level", "The level of light from 0-100", NUMBER,
                          "BrightnessProperty");

ThingProperty deviceRGB("rgb", "The color of light in RGB", INTEGER, "ColorProperty");
// Declare our array of values globally, so the pointer stays valid
ThingPropertyValue arrayValues[3];

bool lastOn = false;
String lastColor = "#ff00ff";

void setup(void) {
  Serial.begin(115200);

  // Create a WiFi station
  WiFi.mode(WIFI_STA);

  // Start WiFi auto-connect routine
  AsyncWiFiManager awm;
  Serial.println("Running autoConnect");

  // Hack for https://github.com/tzapu/WiFiManager/issues/979
  awm.setEnableConfigPortal(false);
  if (!awm.autoConnect()) {
    Serial.println("Retry autoConnect");
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    awm.setEnableConfigPortal(true);
    awm.autoConnect();
  }
  // End of hack

  if (!awm.autoConnect()) {
    Serial.println("Failed to connect");
    ESP.restart();
  } else {
    // Start NeoPixels
    strip.Begin();
    strip.Show();

    // Add extra device metadata
    device.addContext("https://iot.mozilla.org/schemas/");

    // Add Thing properties
    device.addProperty(&deviceOn);

    ThingPropertyValue levelValue;
    levelValue.number = 50;
    deviceLevel.setValue(levelValue);
    device.addProperty(&deviceLevel);

    arrayValues[0].integer = 0;
    arrayValues[1].integer = 50;
    arrayValues[2].integer = 50;
    deviceRGB.setValueArray(arrayValues, 3);
    deviceRGB.minimum = 0;
    deviceRGB.maximum = 255;
    device.addProperty(&deviceRGB);

    // Create our Thing adapter
    adapter = new WebThingAdapter(&device, "NeoPixel Lamp", WiFi.localIP());
    Serial.println("Starting HTTP server");
    // Start the server
    adapter->begin();
    Serial.print("http://");
    Serial.print(WiFi.localIP());
  }
}

void update(int red, int green, int blue, int const level) {
  float dim = level / 100.;

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.SetPixelColor(i, RgbColor(red * dim, green * dim, blue * dim));
  }
  strip.Show();
}

void loop(void) {
  adapter->update();

  bool on = deviceOn.getValue().boolean;
  int level = deviceLevel.getValue().number;

  int r = deviceRGB.getValues()[0].integer;
  int g = deviceRGB.getValues()[1].integer;
  int b = deviceRGB.getValues()[2].integer;
  update(r, g, b, on ? level : 0);
  
  if (on != lastOn) {
    Serial.print(device.id);
    Serial.print(": on: ");
    Serial.print(on);
    Serial.print(", level: ");
    Serial.print(level);
    Serial.print(", color: ");
    Serial.println(lastColor);
  }
  lastOn = on;
}
