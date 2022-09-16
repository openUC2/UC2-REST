#include "config.h"
#include <ArduinoJson.h>
#ifdef IS_MOTOR
#include "src/motor/FocusMotor.h"
#endif
#ifdef IS_LED
#include "src/led/led_controller.h"
#endif
#ifdef IS_LASER
#include "src/laser/LaserController.h"
#endif
#ifdef IS_ANALOG
#include "src/analog/AnalogController.h"
#endif
#include "src/state/State.h"
#ifdef IS_SCANNER
#include "src/scanner/ScannerController.h"
#endif
#ifdef IS_PID
#include "src/pid/PidController.h"
#endif
#ifdef IS_DIGITAL
#include "src/digital/DigitalController.h"
#endif
#ifdef IS_READSENSOR
#include "src/sensor/SensorController.h"
#endif
#if defined IS_DAC || defined IS_DAC_FAKE
#include "src/dac/DacController.h"
#endif
#ifdef IS_SLM
#include "src/slm/SlmController.h"
#endif
#if defined IS_PS4 || defined IS_PS3
#include "src/gamepads/ps_3_4_controller.h"
#endif
#include "src/wifi/WifiController.h"
#include "src/config/ConfigController.h"
#ifdef IS_SERIAL
#include "src/serial/SerialProcess.h"
#endif

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

PINDEF *pins;

void setup()
{
  // Start Serial
  Serial.begin(BAUDRATE);
  Serial.println("Start setup");
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  // for any timing related puposes..
  state.startMillis = millis();
  pins = new PINDEF();
  state.getDefaultPinDef(pins);
  printPinDef();
  wifi.createJsonDoc();
  state.setup(pins, wifi.jsonDocument);

  state.printInfo();
  config.setup(pins, wifi.jsonDocument);
  // if we boot for the first time => reset the preferences! // TODO: Smart? If not, we may have the problem that a wrong pin will block bootup
  if (config.isFirstRun())
  {
    Serial.println("First Run, resetting config");
    config.resetPreferences();
  }
  // check if setup went through after new config - avoid endless boot-loop
  config.checkSetupCompleted();
  printPinDef();
  
  // reset jsonDocument
  wifi.jsonDocument->clear();

  // connect to wifi if necessary
  wifi.setup(pins->mSSID,pins->mPWD);
  //bool isResetWifiSettings = false;
  //wifi.autoconnectWifi(isResetWifiSettings);
  //wifi.setup_routing();
  //wifi.startserver();

  Serial.println(state_act_endpoint);
  Serial.println(state_get_endpoint);
  Serial.println(state_set_endpoint);

#ifdef IS_SLM
  Serial.println("IS_SLM");
  slm.setup(jsonDocument);
#endif
#ifdef IS_LED
  Serial.println("IS_LED");
#ifdef DEBUG_LED
  led.DEBUG = true;
#endif
  led.setup(pins, wifi.jsonDocument);
#endif

#ifdef IS_MOTOR
  Serial.println("IS_MOTOR");
#ifdef DEBUG_MOTOR
  motor.DEBUG = true;
#endif
  motor.setup(pins, wifi.jsonDocument);
#endif

#if defined IS_PS4 || defined IS_PS3
  if (config.isFirstRun())
  {
    state.clearBlueetoothDevice();
    ESP.restart();
  }
#ifdef DEBUG_GAMEPAD
  ps_c.DEBUG = true;
#endif
  ps_c.start();
#endif

#ifdef IS_LASER
  laser.setup(pins, wifi.jsonDocument);
#endif

#if defined IS_DAC || defined IS_DAC_FAKE
#if defined IS_DAC
  Serial.println("IS_DAC");
#endif
#if defined IS_DAC_FAKE
  Serial.println("IS_DAC_FAKE");
#endif
  dac.setup(pins, wifi.jsonDocument);
#endif

#ifdef IS_ANALOG
  Serial.println("IS_ANALOG");
#ifdef DEBUG_ANALOG
  analog->DEBUG = true;
#endif
  analog.setup(pins, wifi.jsonDocument);
#endif

#ifdef IS_DIGITAL
  digital.setup(pins);
#endif

#ifdef IS_READSENSOR
  Serial.println("IS_SENSOR");
  sensor.setup(pins, wifi.jsonDocument);
  Serial.println(readsensor_act_endpoint);
  Serial.println(readsensor_set_endpoint);
  Serial.println(readsensor_get_endpoint);
#endif

#ifdef IS_PID
  Serial.println("IS_PID");
  pid.setup(pins, wifi.jsonDocument);
  Serial.println(PID_act_endpoint);
  Serial.println(PID_set_endpoint);
  Serial.println(PID_get_endpoint);
#endif
#ifdef IS_SCANNER
  Serial.println("IS_SCANNER");
  scanner.setup(pins);
#endif

  Serial.println("End setup");
}

void loop()
{
  // handle any http requests
  wifi.handelMessages();

  // for any timing-related purposes
  state.currentMillis = millis();
#ifdef IS_SERIAL
  serial.loop(wifi.jsonDocument);
#endif
  /*
     continous control during loop
  */
#ifdef IS_LASER
  laser.loop();
#endif

#if defined IS_PS4 || defined IS_PS3
  ps_c.control(); // if controller is operating motors, overheating protection is enabled
#endif
#ifdef IS_WIFI
  wifi.handelMessages();
#endif

  /*
      continous control during loop
  */
  if (!motor.isstop)
  {
    motor.isactive = true;
    motor.background();
  }
#ifdef IS_PID
  if (pid.PID_active && (state.currentMillis - state.startMillis >= pid.PID_updaterate))
  {
    pid.background();
    state.startMillis = millis();
  }
#endif
}

void printPinDef()
{
  Serial.print("Indentifier:");
  Serial.println(pins->identifier_setup);
  Serial.print("analogPin1:");
  Serial.println(pins->analog_PIN_1);
  Serial.print("analogPin2:");
  Serial.println(pins->analog_PIN_2);
  Serial.print("analogPin3:");
  Serial.println(pins->analog_PIN_3);

  Serial.print("STEP_A:");
  Serial.println(pins->STEP_A);
  Serial.print("STEP_X:");
  Serial.println(pins->STEP_X);
  Serial.print("STEP_Y:");
  Serial.println(pins->STEP_Y);
  Serial.print("STEP_Z:");
  Serial.println(pins->STEP_Z);
  Serial.print("DIR_A:");
  Serial.println(pins->DIR_A);
  Serial.print("DIR_X:");
  Serial.println(pins->DIR_X);
  Serial.print("DIR_Y:");
  Serial.println(pins->DIR_Y);
  Serial.print("DIR_Z:");
  Serial.println(pins->DIR_Z);
  Serial.print("ENABLE:");
  Serial.println(pins->ENABLE);

  Serial.print("LASER_PIN_1:");
  Serial.println(pins->LASER_PIN_1);
  Serial.print("LASER_PIN_2:");
  Serial.println(pins->LASER_PIN_2);
  Serial.print("LASER_PIN_3:");
  Serial.println(pins->LASER_PIN_3);

  Serial.print("digital_PIN_1:");
  Serial.println(pins->digital_PIN_1);
  Serial.print("digital_PIN_2:");
  Serial.println(pins->digital_PIN_2);
  Serial.print("digital_PIN_3:");
  Serial.println(pins->digital_PIN_3);

  Serial.print("LED_ARRAY_PIN:");
  Serial.println(pins->LED_ARRAY_PIN);
  Serial.print("LED_ARRAY_NUM:");
  Serial.println(pins->LED_ARRAY_NUM);

  Serial.print("dac_fake_1:");
  Serial.println(pins->dac_fake_1);
  Serial.print("dac_fake_2:");
  Serial.println(pins->dac_fake_2);

  Serial.print("ADC_pin_0:");
  Serial.println(pins->ADC_pin_0);
  Serial.print("ADC_pin_1:");
  Serial.println(pins->ADC_pin_1);
  Serial.print("ADC_pin_2:");
  Serial.println(pins->ADC_pin_2);
}
