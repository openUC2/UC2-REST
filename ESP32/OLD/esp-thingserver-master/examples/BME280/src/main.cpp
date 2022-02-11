#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

// Make sure we use an async web server for the WiFi manager
#define WM_ASYNC
#include <AsyncWiFiManager.h>

#include <Thing.h>
#include <WebThingAdapter.h>

Adafruit_BME280 bme;
WebThingAdapter *adapter;

// Define basic Thing attributes
const char *bme280Types[] = {"TemperatureSensor", "HumiditySensor", "BarometricPressureSensor", nullptr};
ThingDevice weather("bme280", "BME280 Weather Sensor", bme280Types);

ThingProperty weatherTemp("temperature", "", NUMBER, "TemperatureProperty");
ThingProperty weatherHum("humidity", "", NUMBER, "HumidityProperty");
ThingProperty weatherPres("pressure", "", NUMBER, "BarometricPressureProperty");

void readBME280Data() {
  ThingPropertyValue value;

  value.number = bme.readPressure() / 100.0F;
  weatherPres.setValue(value);
  value.number = bme.readTemperature();
  weatherTemp.setValue(value);
  value.number = bme.readHumidity();
  weatherHum.setValue(value);
}

void setup() {
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
    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Default BME settings
    unsigned status;
    status = bme.begin(BME280_ADDRESS_ALTERNATE);  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }

    // Add Thing properties
    weatherTemp.unit = "celsius";
    weather.addProperty(&weatherTemp);
    weatherPres.unit = "hPa";
    weather.addProperty(&weatherPres);
    weatherHum.unit = "percent";
    weather.addProperty(&weatherHum);

    // Create our Thing adapter
    adapter = new WebThingAdapter(&weather, "weathersensor", WiFi.localIP());
    // Start the server
    adapter->begin();
    Serial.println("HTTP server started");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
  }
}

void loop() {
  readBME280Data();
  adapter->update();
}
