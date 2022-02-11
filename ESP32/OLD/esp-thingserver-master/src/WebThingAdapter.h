/**
 * WebThingAdapter.h
 *
 * Exposes the Web Thing API based on provided ThingDevices.
 * Suitable for ESP32 and ESP8266 using ESPAsyncWebServer and ESPAsyncTCP
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Thing.h"

#define ESP_MAX_PUT_BODY_SIZE 512

class WebThingAdapter {
public:
  WebThingAdapter(ThingDevice *_thing, String _name, IPAddress _ip,
                  uint16_t _port = 80);

  void begin();
  void update();

private:
  ThingDevice *thing = nullptr;
  AsyncWebServer server;
  String name;
  String ip;
  uint16_t port;

  char body_data[ESP_MAX_PUT_BODY_SIZE];
  bool b_has_body_data = false;

  bool verifyHost(AsyncWebServerRequest *request);

#ifndef WITHOUT_WS
  void sendErrorMsg(DynamicJsonDocument &prop, AsyncWebSocketClient &client,
                    int status, const char *msg);

  void handleWS(AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, const uint8_t *rawData,
                size_t len, ThingDevice *device);

  void sendChangedProperties(ThingDevice *device);
#endif

  void handleUnknown(AsyncWebServerRequest *request);
  void handleOptions(AsyncWebServerRequest *request);

  void handleThing(AsyncWebServerRequest *request, ThingDevice *&device);

  void handleThingPropertyGet(AsyncWebServerRequest *request,
                              ThingItem *item);

  void handleThingActionGet(AsyncWebServerRequest *request,
                            ThingDevice *device, ThingAction *action);

  void handleThingActionDelete(AsyncWebServerRequest *request,
                               ThingDevice *device, ThingAction *action);

  void handleThingActionPost(AsyncWebServerRequest *request,
                             ThingDevice *device, ThingAction *action);

  void handleThingEventGet(AsyncWebServerRequest *request, ThingDevice *device,
                           ThingItem *item);

  void handleThingPropertiesGet(AsyncWebServerRequest *request,
                                ThingItem *rootItem);

  void handleThingActionsGet(AsyncWebServerRequest *request,
                             ThingDevice *device);

  void handleThingEventsGet(AsyncWebServerRequest *request,
                            ThingDevice *device);

  void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len,
                  size_t index, size_t total);

  void handleThingPropertyPut(AsyncWebServerRequest *request,
                              ThingDevice *device, ThingProperty *property);
};
