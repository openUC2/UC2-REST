#include "Thing.h"

ThingActionObject::ThingActionObject(const char *name_,
                                     DynamicJsonDocument *actionRequest_,
                                     void (*start_fn_)(const JsonVariant &),
                                     void (*cancel_fn_)())
    : start_fn(start_fn_), cancel_fn(cancel_fn_), name(name_),
      actionRequest(actionRequest_),
      timeRequested("1970-01-01T00:00:00+00:00"), status("created") {
  generateId();
  timeRequested = getTimeStampString();
}

#ifndef WITHOUT_WS
void ThingActionObject::setNotifyFunction(
    std::function<void(ThingActionObject *)> notify_fn_) {
  notify_fn = notify_fn_;
}
#endif

void ThingActionObject::generateId() {
  for (uint8_t i = 0; i < 16; ++i) {
    char c = (char)random('0', 'g');

    if (c > '9' && c < 'a') {
      --i;
      continue;
    }

    id += c;
  }
}

void ThingActionObject::serialize(JsonObject obj, String deviceId) {
  JsonObject actionRequestObj = actionRequest->as<JsonObject>();
  obj["input"] = actionRequestObj;

  obj["status"] = status;
  obj["timeRequested"] = timeRequested;

  if (timeCompleted != "") {
    obj["timeCompleted"] = timeCompleted;
  }

  obj["id"] = id;
  obj["title"] = name;
  obj["href"] = "/actions/" + name + "/" + id;
}

void ThingActionObject::setStatus(const char *s) {
  status = s;

#ifndef WITHOUT_WS
  if (notify_fn != nullptr) {
    notify_fn(this);
  }
#endif
}

void ThingActionObject::start() {
  this->setStatus("pending");

  JsonObject actionRequestObj = actionRequest->as<JsonObject>();
  start_fn(actionRequestObj);

  finish();
}

void ThingActionObject::cancel() {
  if (cancel_fn != nullptr) {
    cancel_fn();
  }
}

void ThingActionObject::finish() {
  timeCompleted = getTimeStampString();
  this->setStatus("completed");
}

ThingAction::ThingAction(const char *id_, ThingActionObject *(*generator_fn_)(
                                              DynamicJsonDocument *))
    : generator_fn(generator_fn_), id(id_) {}

ThingAction::ThingAction(
    const char *id_, JsonObject *input_,
    ThingActionObject *(*generator_fn_)(DynamicJsonDocument *))
    : generator_fn(generator_fn_), id(id_), input(input_) {}

ThingAction::ThingAction(
    const char *id_, const char *title_, const char *description_,
    const char *type_, JsonObject *input_,
    ThingActionObject *(*generator_fn_)(DynamicJsonDocument *))
    : generator_fn(generator_fn_), id(id_), title(title_),
      description(description_), type(type_), input(input_) {}

void ThingAction::serialize(JsonObject obj, String deviceId,
                            String resourceType) {
  if (title != "") {
    obj["title"] = title;
  }

  if (description != "") {
    obj["description"] = description;
  }

  if (type != "") {
    obj["@type"] = type;
  }

  if (input != nullptr) {
    JsonObject inputObj = obj.createNestedObject("input");
    for (JsonPair kv : *input) {
      inputObj[kv.key()] = kv.value();
    }
  }

  // 2.11 Action object: A links array (An array of Link objects linking
  // to one or more representations of an Action resource, each with an
  // implied default rel=action.)
  JsonArray inline_links = obj.createNestedArray("links");
  JsonObject inline_links_prop = inline_links.createNestedObject();
  inline_links_prop["href"] = "/actions/" + id;
}

ThingActionObject *ThingAction::create(DynamicJsonDocument *actionRequest) {
  return generator_fn(actionRequest);
}

ThingItem::ThingItem(const char *id_, const char *description_,
                     ThingDataType type_, const char *atType_)
    : id(id_), description(description_), type(type_), atType(atType_) {}

void ThingItem::setValue(ThingDataValue newValue) {
  // Set the items value to a ThingDataValue instance
  this->value = newValue;
  this->hasChanged = true;
}

void ThingItem::setValueArray(ThingDataValue newValues[], int n) {
  // Set the items values to an array of ThingDataValue instances
  this->values = newValues;
  this->hasChanged = true;
  this->_isArray = true;
  this->_arrayLength = n;
}

void ThingItem::setValue(unsigned int index, ThingDataValue newValue) {
  // Set an element of the items values array to a ThingDataValue instance
  this->values[index] = newValue;
}

void ThingItem::setValue(const char *s) {
  // Set the items value to a string
  *(this->getValue().string) = s;
  this->hasChanged = true;
}

void ThingItem::setValue(unsigned int index, const char *s) {
  // Set an element of the items values array to a string
  if (this->isArray() && index < this->arrayLength()) {
    *(this->values[index].string) = s;
    this->hasChanged = true;
  }
}

/**
 * Returns the property value if it has been changed via {@link setValue}
 * since the last call or returns a nullptr.
 */
ThingDataValue *ThingItem::changedValueOrNull() {
  ThingDataValue *v = this->hasChanged ? &this->value : nullptr;
  this->hasChanged = false;
  return v;
}

ThingDataValue ThingItem::getValue() { return this->value; }
ThingDataValue *ThingItem::getValues() { return this->values; }

bool ThingItem::isArray() { return this->_isArray; }
int ThingItem::arrayLength() { return this->_arrayLength; }

void ThingItem::serialize(JsonObject rootObj, String deviceId,
                          String resourceType) {
  JsonObject obj;

  if (this->isArray()) {
    rootObj["type"] = "array";
    rootObj["minItems"] = this->arrayLength();
    rootObj["maxItems"] = this->arrayLength();
    obj = rootObj.createNestedObject("items");
  } else {
    obj = rootObj;
  }

  if (readOnly) {
    rootObj["readOnly"] = true;
  }

  if (title != "") {
    rootObj["title"] = title;
  }

  if (description != "") {
    rootObj["description"] = description;
  }

  if (atType != nullptr) {
    rootObj["@type"] = atType;
  }

  switch (type) {
  case NO_STATE:
    break;
  case BOOLEAN:
    obj["type"] = "boolean";
    break;
  case NUMBER:
    obj["type"] = "number";
    break;
  case INTEGER:
    obj["type"] = "integer";
    break;
  case STRING:
    obj["type"] = "string";
    break;
  }

  if (unit != "") {
    obj["unit"] = unit;
  }

  if (minimum < maximum) {
    obj["minimum"] = minimum;
  }

  if (maximum > minimum) {
    obj["maximum"] = maximum;
  }

  if (multipleOf > 0) {
    obj["multipleOf"] = multipleOf;
  }

  // 2.9 Property object: A links array (An array of Link objects linking
  // to one or more representations of a Property resource, each with an
  // implied default rel=property.)
  JsonArray inline_links = rootObj.createNestedArray("links");
  JsonObject inline_links_prop = inline_links.createNestedObject();
  inline_links_prop["href"] = "/" + resourceType + "/" + id;
}

void ThingItem::serializeValueToObject(JsonObject prop) {
  DynamicJsonDocument doc(SMALL_JSON_DOCUMENT_SIZE);
  JsonVariant variant = doc.to<JsonVariant>();
  this->serializeValueToVariant(variant);
  prop[this->id] = variant;
}

void ThingItem::serializeValueToVariant(JsonVariant variant) {
  if (this->isArray()) {
    JsonArray variantArray = variant.to<JsonArray>();
    ThingDataValue *valueArray = this->getValues();

    for (unsigned int a = 0; a < this->arrayLength(); a++) {
      ThingDataValue dataValue = valueArray[a];
      switch (this->type) {
      case NO_STATE:
        break;
      case BOOLEAN:
        variantArray.add(dataValue.boolean);
        break;
      case NUMBER:
        variantArray.add(dataValue.number);
        break;
      case INTEGER:
        variantArray.add(dataValue.integer);
        break;
      case STRING:
        variantArray.add(*dataValue.string);
        break;
      }
    }
  } else {
    switch (this->type) {
    case NO_STATE:
      break;
    case BOOLEAN:
      variant.set(this->getValue().boolean);
      break;
    case NUMBER:
      variant.set(this->getValue().number);
      break;
    case INTEGER:
      variant.set(this->getValue().integer);
      break;
    case STRING:
      variant.set(*this->getValue().string);
      break;
    }
  }
}

ThingProperty::ThingProperty(const char *id_, const char *description_,
                             ThingDataType type_, const char *atType_,
                             void (*callback_)(ThingPropertyValue))
    : ThingItem(id_, description_, type_, atType_), callback(callback_) {}

void ThingProperty::serialize(JsonObject obj, String deviceId,
                              String resourceType) {
  ThingItem::serialize(obj, deviceId, resourceType);

  const char **enumVal = propertyEnum;
  bool hasEnum = propertyEnum != nullptr && *propertyEnum != nullptr;

  if (hasEnum) {
    enumVal = propertyEnum;
    JsonArray propEnum = obj.createNestedArray("enum");
    while (propertyEnum != nullptr && *enumVal != nullptr) {
      propEnum.add(*enumVal);
      enumVal++;
    }
  }
}

void ThingProperty::changed(ThingPropertyValue newValue) {
  if (callback != nullptr) {
    callback(newValue);
  }
}

#ifndef WITHOUT_WS

EventSubscription::EventSubscription(uint32_t id_) : id(id_) {}

ThingEvent::ThingEvent(const char *id_, const char *description_,
                       ThingDataType type_, const char *atType_)
    : ThingItem(id_, description_, type_, atType_) {}

void ThingEvent::addSubscription(uint32_t id) {
  EventSubscription *sub = new EventSubscription(id);
  sub->next = subscriptions;
  subscriptions = sub;
}

void ThingEvent::removeSubscription(uint32_t id) {
  EventSubscription *curr = subscriptions;
  EventSubscription *prev = nullptr;
  while (curr != nullptr) {
    if (curr->id == id) {
      if (prev == nullptr) {
        subscriptions = curr->next;
      } else {
        prev->next = curr->next;
      }

      delete curr;
      return;
    }

    prev = curr;
    curr = curr->next;
  }
}

bool ThingEvent::isSubscribed(uint32_t id) {
  EventSubscription *curr = subscriptions;
  while (curr != nullptr) {
    if (curr->id == id) {
      return true;
    }
    curr = curr->next;
  }
  return false;
}

#endif

ThingEventObject::ThingEventObject(const char *name_, ThingDataType type_,
                                   ThingDataValue value_)
    : name(name_), type(type_), value(value_),
      timestamp("1970-01-01T00:00:00+00:00") {}

ThingEventObject::ThingEventObject(const char *name_, ThingDataType type_,
                                   ThingDataValue value_, String timestamp_)
    : name(name_), type(type_), value(value_), timestamp(timestamp_) {}

ThingDataValue ThingEventObject::getValue() { return this->value; }

void ThingEventObject::serialize(JsonObject obj) {
  JsonObject data = obj.createNestedObject(name);
  switch (this->type) {
  case NO_STATE:
    break;
  case BOOLEAN:
    data["data"] = this->getValue().boolean;
    break;
  case NUMBER:
    data["data"] = this->getValue().number;
    break;
  case INTEGER:
    data["data"] = this->getValue().integer;
    break;
  case STRING:
    data["data"] = *this->getValue().string;
    break;
  }

  data["timestamp"] = timestamp;
}

ThingDevice::ThingDevice(const char *_id, const char *_title,
                         const char **_type)
    : id(_id), title(_title), type(_type),
      contextDoc(SMALL_JSON_DOCUMENT_SIZE) {
  contextArray = contextDoc.to<JsonArray>();
  contextArray.add("https://www.w3.org/2019/wot/td/v1");
  nestedContextObj = contextArray.createNestedObject();
}

ThingDevice::~ThingDevice() {
#if !defined(WITHOUT_WS) && (defined(ESP8266) || defined(ESP32))
  if (ws)
    delete ws;
#endif
}

#ifndef WITHOUT_WS

void ThingDevice::removeEventSubscriptions(uint32_t id) {
  ThingEvent *event = firstEvent;
  while (event != nullptr) {
    event->removeSubscription(id);
    event = (ThingEvent *)event->next;
  }
}

void ThingDevice::addEventSubscription(uint32_t id, String eventName) {
  ThingEvent *event = findEvent(eventName.c_str());
  if (!event) {
    return;
  }

  event->addSubscription(id);
}

void ThingDevice::sendActionStatus(ThingActionObject *action) {
  DynamicJsonDocument message(LARGE_JSON_DOCUMENT_SIZE);
  message["messageType"] = "actionStatus";
  JsonObject prop = message.createNestedObject("data");
  action->serialize(prop, id);
  String jsonStr;
  serializeJson(message, jsonStr);
  // Inform all connected ws clients about action statuses
  ((AsyncWebSocket *)ws)->textAll(jsonStr);
}

#endif

void ThingDevice::addContext(String url, String prefix) {
  nestedContextObj[prefix] = url;
}

void ThingDevice::addContext(String url) { contextArray.add(url); }

ThingProperty *ThingDevice::findProperty(const char *id) {
  ThingProperty *p = this->firstProperty;
  while (p) {
    if (!strcmp(p->id.c_str(), id))
      return p;
    p = (ThingProperty *)p->next;
  }
  return nullptr;
}

void ThingDevice::addProperty(ThingProperty *property) {
  property->next = firstProperty;
  firstProperty = property;
}

ThingAction *ThingDevice::findAction(const char *id) {
  ThingAction *a = this->firstAction;
  while (a) {
    if (!strcmp(a->id.c_str(), id))
      return a;
    a = (ThingAction *)a->next;
  }
  return nullptr;
}

ThingActionObject *ThingDevice::findActionObject(const char *id) {
  ThingActionObject *a = this->actionQueue;
  while (a) {
    if (!strcmp(a->id.c_str(), id))
      return a;
    a = a->next;
  }
  return nullptr;
}

void ThingDevice::addAction(ThingAction *action) {
  action->next = firstAction;
  firstAction = action;
}

ThingEvent *ThingDevice::findEvent(const char *id) {
  ThingEvent *e = this->firstEvent;
  while (e) {
    if (!strcmp(e->id.c_str(), id))
      return e;
    e = (ThingEvent *)e->next;
  }
  return nullptr;
}

void ThingDevice::addEvent(ThingEvent *event) {
  event->next = firstEvent;
  firstEvent = event;
}

void ThingDevice::setProperty(const char *name, const JsonVariant &newValue) {
  ThingProperty *property = findProperty(name);

  // If the property doesn't exist, return immediately
  if (property == nullptr) {
    return;
  }

  // If the property is an array
  if (property->isArray()) {
    // If the property is an array but the input JSON isn't
    if (!newValue.is<JsonArray>()) {
      // Return immediately
      return;
    }

    // Create a JSON array from the input variant
    JsonArray variantArray = newValue.as<JsonArray>();
    // If input and property arrays are different lengths
    if (variantArray.size() != property->arrayLength()) {
      // Return immediately
      return;
    }
    // For each element in the property array
    for (unsigned int a = 0; a < property->arrayLength(); a++) {
      switch (property->type) {
      case NO_STATE: {
        break;
      }
      case BOOLEAN: {
        ThingDataValue value;
        value.boolean = variantArray[a].as<bool>();
        property->setValue(a, value);
        property->changed(value);
        break;
      }
      case NUMBER: {
        ThingDataValue value;
        value.number = variantArray[a].as<double>();
        property->setValue(a, value);
        property->changed(value);
        break;
      }
      case INTEGER: {
        ThingDataValue value;
        value.integer = variantArray[a].as<signed long long>();
        property->setValue(a, value);
        property->changed(value);
        break;
      }
      case STRING:
        property->setValue(a, variantArray[a].as<const char *>());
        property->changed(property->getValue());
        break;
      }
    }
  }

  // Is the property is a single value
  else {
    switch (property->type) {
    case NO_STATE: {
      break;
    }
    case BOOLEAN: {
      ThingDataValue value;
      value.boolean = newValue.as<bool>();
      property->setValue(value);
      property->changed(value);
      break;
    }
    case NUMBER: {
      ThingDataValue value;
      value.number = newValue.as<double>();
      property->setValue(value);
      property->changed(value);
      break;
    }
    case INTEGER: {
      ThingDataValue value;
      value.integer = newValue.as<signed long long>();
      property->setValue(value);
      property->changed(value);
      break;
    }
    case STRING:
      property->setValue(newValue.as<const char *>());
      property->changed(property->getValue());
      break;
    }
  }
}

ThingActionObject *
ThingDevice::requestAction(const char *name,
                           DynamicJsonDocument *actionRequest) {

  ThingAction *action = findAction(name);
  if (action == nullptr) {
    return nullptr;
  }

  ThingActionObject *obj = action->create(actionRequest);
  if (obj == nullptr) {
    return nullptr;
  }

  queueActionObject(obj);
  return obj;
}

void ThingDevice::removeAction(String id) {
  ThingActionObject *curr = actionQueue;
  ThingActionObject *prev = nullptr;
  while (curr != nullptr) {
    if (curr->id == id) {
      if (prev == nullptr) {
        actionQueue = curr->next;
      } else {
        prev->next = curr->next;
      }

      curr->cancel();
      delete curr->actionRequest;
      delete curr;
      return;
    }

    prev = curr;
    curr = curr->next;
  }
}

void ThingDevice::queueActionObject(ThingActionObject *obj) {
  obj->next = actionQueue;
  actionQueue = obj;
}

void ThingDevice::queueEventObject(ThingEventObject *obj) {
  obj->next = eventQueue;
  eventQueue = obj;

#ifndef WITHOUT_WS
  ThingEvent *event = findEvent(obj->name.c_str());
  if (!event) {
    return;
  }

  // * Send events as defined in "4.7 event message"
  DynamicJsonDocument message(SMALL_JSON_DOCUMENT_SIZE);
  message["messageType"] = "event";
  JsonObject data = message.createNestedObject("data");
  obj->serialize(data);
  String jsonStr;
  serializeJson(message, jsonStr);

  // Inform all subscribed ws clients about events
  for (AsyncWebSocketClient *client :
       ((AsyncWebSocket *)this->ws)->getClients()) {
    uint32_t id = client->id();

    if (event->isSubscribed(id)) {
      ((AsyncWebSocket *)this->ws)->text(id, jsonStr);
    }
  }
#endif
}

void ThingDevice::serialize(JsonObject descr, String ip, uint16_t port) {
  descr["id"] = this->id;
  descr["title"] = this->title;
  descr["@context"] = contextArray;

  if (this->description != "") {
    descr["description"] = this->description;
  }
  if (port != 80) {
    char buffer[33];
    itoa(port, buffer, 10);
    descr["base"] = "http://" + ip + ":" + buffer + "/";
  } else {
    descr["base"] = "http://" + ip + "/";
  }

  JsonObject securityDefinitions =
      descr.createNestedObject("securityDefinitions");
  JsonObject nosecSc = securityDefinitions.createNestedObject("nosec_sc");
  nosecSc["scheme"] = "nosec";
  descr["security"] = "nosec_sc";

  JsonArray typeJson = descr.createNestedArray("@type");
  const char **type = this->type;
  while ((*type) != nullptr) {
    typeJson.add(*type);
    type++;
  }

  JsonArray links = descr.createNestedArray("links");
  {
    JsonObject links_prop = links.createNestedObject();
    links_prop["rel"] = "properties";
    links_prop["href"] = "/properties";
  }

  {
    JsonObject links_prop = links.createNestedObject();
    links_prop["rel"] = "actions";
    links_prop["href"] = "/actions";
  }

  {
    JsonObject links_prop = links.createNestedObject();
    links_prop["rel"] = "events";
    links_prop["href"] = "/events";
  }

#ifndef WITHOUT_WS
  {
    JsonObject links_prop = links.createNestedObject();
    links_prop["rel"] = "alternate";

    if (port != 80) {
      char buffer[33];
      itoa(port, buffer, 10);
      links_prop["href"] = "ws://" + ip + ":" + buffer + "/things/" + this->id;
    } else {
      links_prop["href"] = "ws://" + ip + "/things/" + this->id;
    }
  }
#endif

  ThingProperty *property = this->firstProperty;
  if (property != nullptr) {
    JsonObject properties = descr.createNestedObject("properties");
    while (property != nullptr) {
      JsonObject obj = properties.createNestedObject(property->id);
      property->serialize(obj, id, "properties");
      property = (ThingProperty *)property->next;
    }
  }

  ThingAction *action = this->firstAction;
  if (action != nullptr) {
    JsonObject actions = descr.createNestedObject("actions");
    while (action != nullptr) {
      JsonObject obj = actions.createNestedObject(action->id);
      action->serialize(obj, id, "actions");
      action = action->next;
    }
  }

  ThingEvent *event = this->firstEvent;
  if (event != nullptr) {
    JsonObject events = descr.createNestedObject("events");
    while (event != nullptr) {
      JsonObject obj = events.createNestedObject(event->id);
      event->serialize(obj, id, "events");
      event = (ThingEvent *)event->next;
    }
  }
}

void ThingDevice::serializeActionQueue(JsonArray array) {
  ThingActionObject *curr = actionQueue;
  while (curr != nullptr) {
    JsonObject action = array.createNestedObject();
    curr->serialize(action, id);
    curr = curr->next;
  }
}

void ThingDevice::serializeActionQueue(JsonArray array, String name) {
  ThingActionObject *curr = actionQueue;
  while (curr != nullptr) {
    if (curr->name == name) {
      JsonObject action = array.createNestedObject();
      curr->serialize(action, id);
    }
    curr = curr->next;
  }
}

void ThingDevice::serializeEventQueue(JsonArray array) {
  ThingEventObject *curr = eventQueue;
  while (curr != nullptr) {
    JsonObject event = array.createNestedObject();
    curr->serialize(event);
    curr = curr->next;
  }
}

void ThingDevice::serializeEventQueue(JsonArray array, String name) {
  ThingEventObject *curr = eventQueue;
  while (curr != nullptr) {
    if (curr->name == name) {
      JsonObject event = array.createNestedObject();
      curr->serialize(event);
    }
    curr = curr->next;
  }
}