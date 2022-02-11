/**
 * Thing.h
 *
 * Provides ThingProperty and ThingDevice classes for creating modular Web
 * Things.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "TimeUtils.h"

#if !defined(ESP8266) && !defined(ESP32) && !defined(WITHOUT_WS)
#define WITHOUT_WS 1
#endif

#if !defined(WITHOUT_WS) && (defined(ESP8266) || defined(ESP32))
#include <ESPAsyncWebServer.h>
#endif

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#ifndef LARGE_JSON_DOCUMENT_SIZE
#ifdef LARGE_JSON_BUFFERS
#define LARGE_JSON_DOCUMENT_SIZE 4096
#else
#define LARGE_JSON_DOCUMENT_SIZE 1024
#endif
#endif

#ifndef SMALL_JSON_DOCUMENT_SIZE
#ifdef LARGE_JSON_BUFFERS
#define SMALL_JSON_DOCUMENT_SIZE 1024
#else
#define SMALL_JSON_DOCUMENT_SIZE 256
#endif
#endif

enum ThingDataType { NO_STATE, BOOLEAN, NUMBER, INTEGER, STRING };
typedef ThingDataType ThingPropertyType;

union ThingDataValue {
  bool boolean;
  double number;
  signed long long integer;
  String *string;
};
typedef ThingDataValue ThingPropertyValue;

class ThingActionObject {
private:
  void (*start_fn)(const JsonVariant &);
  void (*cancel_fn)();

#ifndef WITHOUT_WS
  std::function<void(ThingActionObject *)> notify_fn;
#endif

public:
  String name;
  DynamicJsonDocument *actionRequest = nullptr;
  String timeRequested;
  String timeCompleted;
  String status;
  String id;
  ThingActionObject *next = nullptr;

  ThingActionObject(const char *name_, DynamicJsonDocument *actionRequest_,
                    void (*start_fn_)(const JsonVariant &),
                    void (*cancel_fn_)());

#ifndef WITHOUT_WS
  void setNotifyFunction(std::function<void(ThingActionObject *)> notify_fn_);
#endif

  void generateId();
  void serialize(JsonObject obj, String deviceId);
  void setStatus(const char *s);

  void start();
  void cancel();
  void finish();
};

class ThingAction {
private:
  ThingActionObject *(*generator_fn)(DynamicJsonDocument *);

public:
  String id;
  String title;
  String description;
  String type;
  JsonObject *input;
  ThingAction *next = nullptr;

  ThingAction(const char *id_,
              ThingActionObject *(*generator_fn_)(DynamicJsonDocument *));

  ThingAction(const char *id_, JsonObject *input_,
              ThingActionObject *(*generator_fn_)(DynamicJsonDocument *));

  ThingAction(const char *id_, const char *title_, const char *description_,
              const char *type_, JsonObject *input_,
              ThingActionObject *(*generator_fn_)(DynamicJsonDocument *));

  void serialize(JsonObject obj, String deviceId, String resourceType);

  ThingActionObject *create(DynamicJsonDocument *actionRequest);
};

class ThingItem {
public:
  String id;
  String description;
  ThingDataType type;
  String atType;
  ThingItem *next = nullptr;

  bool readOnly = false;
  String unit = "";
  String title = "";
  double minimum = 0;
  double maximum = -1;
  double multipleOf = -1;

  ThingItem(const char *id_, const char *description_, ThingDataType type_,
            const char *atType_);

  void setValue(ThingDataValue newValue);
  void setValue(unsigned int index, ThingDataValue newValue);
  void setValue(const char *s);
  void setValue(unsigned int index, const char *s);
  void setValueArray(ThingDataValue newValues[], int n);

  ThingDataValue *changedValueOrNull();
  ThingDataValue getValue();
  ThingDataValue *getValues();

  bool isArray();
  int arrayLength();

  void serialize(JsonObject rootObj, String deviceId, String resourceType);
  void serializeValueToObject(JsonObject prop);
  void serializeValueToVariant(JsonVariant variant);

private:
  ThingDataValue value = {false};
  ThingDataValue *values;
  bool _isArray = false;
  int _arrayLength = 0;
  bool hasChanged = false;
};

class ThingProperty : public ThingItem {
private:
  void (*callback)(ThingPropertyValue);

public:
  const char **propertyEnum = nullptr;

  ThingProperty(const char *id_, const char *description_, ThingDataType type_,
                const char *atType_,
                void (*callback_)(ThingPropertyValue) = nullptr);

  void serialize(JsonObject obj, String deviceId, String resourceType);
  void changed(ThingPropertyValue newValue);
};

#ifndef WITHOUT_WS
class EventSubscription {
public:
  uint32_t id;
  EventSubscription *next;
  EventSubscription(uint32_t id_);
};

class ThingEvent : public ThingItem {
private:
  EventSubscription *subscriptions = nullptr;

public:
  ThingEvent(const char *id_, const char *description_, ThingDataType type_,
             const char *atType_);

  void addSubscription(uint32_t id);
  void removeSubscription(uint32_t id);
  bool isSubscribed(uint32_t id);
};
#else
using ThingEvent = ThingItem;
#endif

class ThingEventObject {
public:
  String name;
  ThingDataType type;
  ThingDataValue value = {false};
  String timestamp;
  ThingEventObject *next = nullptr;

  ThingEventObject(const char *name_, ThingDataType type_,
                   ThingDataValue value_);
  ThingEventObject(const char *name_, ThingDataType type_,
                   ThingDataValue value_, String timestamp_);

  ThingDataValue getValue();

  void serialize(JsonObject obj);
};

class ThingDevice {
public:
  String id;
  String title;
  String description;
  const char **type;
#if !defined(WITHOUT_WS) && (defined(ESP8266) || defined(ESP32))
  AsyncWebSocket *ws = nullptr;
#endif
  ThingDevice *next = nullptr;
  ThingProperty *firstProperty = nullptr;
  ThingAction *firstAction = nullptr;
  ThingActionObject *actionQueue = nullptr;
  ThingEvent *firstEvent = nullptr;
  ThingEventObject *eventQueue = nullptr;

  DynamicJsonDocument contextDoc;
  JsonArray contextArray;
  JsonObject nestedContextObj;

  ThingDevice(const char *_id, const char *_title, const char **_type);
  ~ThingDevice();

#ifndef WITHOUT_WS
  void removeEventSubscriptions(uint32_t id);
  void addEventSubscription(uint32_t id, String eventName);
  void sendActionStatus(ThingActionObject *action);
#endif

  // Add a context member with a prefix
  // E.g. device.addContext("https://w3id.org/saref#", "saref");
  void addContext(String url, String prefix);
  // Add a context member without a prefix
  // E.g. device.addContext("https://w3id.org/saref#");
  void addContext(String url);

  ThingProperty *findProperty(const char *id);
  void addProperty(ThingProperty *property);

  ThingAction *findAction(const char *id);
  ThingActionObject *findActionObject(const char *id);
  void addAction(ThingAction *action);

  ThingEvent *findEvent(const char *id);
  void addEvent(ThingEvent *event);

  void setProperty(const char *name, const JsonVariant &newValue);

  ThingActionObject *requestAction(const char *name,
                                   DynamicJsonDocument *actionRequest);
  void removeAction(String id);

  void queueActionObject(ThingActionObject *obj);
  void queueEventObject(ThingEventObject *obj);

  // Serialize Thing Description
  void serialize(JsonObject descr, String ip, uint16_t port);

  // Serialize full action queue
  void serializeActionQueue(JsonArray array);
  // Serialize action queue for a particular action name
  void serializeActionQueue(JsonArray array, String name);
  // Serialize full event queue
  void serializeEventQueue(JsonArray array);
  // Serialize event queue for a particular event name
  void serializeEventQueue(JsonArray array, String name);
};
