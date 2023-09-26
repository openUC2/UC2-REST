#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = GPIO_NUM_25;     
     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// PID constants
const float Kp = 2.0;
const float Ki = 0.5;
const float Kd = 1.0;

// PID variables
float previousError = 0;
float integral = 0;

// Desired temperature
const float setpoint = 36.0;  // for example, 50°C

// LEDC variables
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
const int heaterPin = GPIO_NUM_12;  // PWM pin connected to the heater

// Get the current temperature
// Here you'll want to replace this with your actual temperature reading logic
float getTemperature() {
  // Dummy implementation, replace with your sensor's logic
  sensors.requestTemperatures(); 
  return sensors.getTempCByIndex(0);  
}

void setup() {
  Serial.begin(115200);
  sensors.begin();
  // Setup LEDC for heater control
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(heaterPin, ledChannel);
}

void loop() {

  float currentTemperature = getTemperature();
  float error = setpoint - currentTemperature;

  integral += error;
  float derivative = error - previousError;

  // Calculate PID output
  float output = Kp * error + Ki * integral + Kd * derivative;

  // Ensure output is within the range [0, 255] for the 8-bit PWM resolution
  output = constrain(output, 0, 255);

  // Set the PWM signal
  ledcWrite(ledChannel, (int)output);

  // Update the previous error
  previousError = error;

  // Print values for debugging (optional)
  Serial.print("Current Temperature: "); Serial.print(currentTemperature);
  Serial.print(" °C | PWM Output: "); Serial.println(output);

  delay(1000);
}
