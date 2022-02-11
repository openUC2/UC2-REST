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
ThingProperty deviceColor("color", "The color of light in RGB", STRING,
                          "ColorProperty");

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

    ThingPropertyValue colorValue;
    colorValue.string = &lastColor; // default color is white
    deviceColor.setValue(colorValue);
    device.addProperty(&deviceColor);

    // Create our Thing adapter
    adapter = new WebThingAdapter(&device, "NeoPixel Lamp", WiFi.localIP());
    Serial.println("Starting HTTP server");
    // Start the server
    adapter->begin();
    Serial.print("http://");
    Serial.print(WiFi.localIP());
  }
}

void update(String *color, int const level) {
  if (!color)
    return;
  float dim = level / 100.;
  int red, green, blue;
  if (color && (color->length() == 7) && color->charAt(0) == '#') {
    const char *hex = 1 + (color->c_str()); // skip leading '#'
    sscanf(0 + hex, "%2x", &red);
    sscanf(2 + hex, "%2x", &green);
    sscanf(4 + hex, "%2x", &blue);
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.SetPixelColor(i, RgbColor(red * dim, green * dim, blue * dim));
  }
  strip.Show();
}

void loop(void) {
  adapter->update();

  bool on = deviceOn.getValue().boolean;
  int level = deviceLevel.getValue().number;
  update(&lastColor, on ? level : 0);
  
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