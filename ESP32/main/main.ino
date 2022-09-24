#include "config.h"
#include <ArduinoJson.h>
#include "esp_log.h"
#ifdef IS_MOTOR
#include "src/motor/FocusMotor.h"
#endif
#ifdef IS_LED
#include "src/led/LedController.h"
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

static const char *TAG = "Main.ino";
void setup()
{
  // Start Serial
  Serial.begin(BAUDRATE);
  delay(500);
  ESP_LOGI(TAG,"Start setup");
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  // for any timing related puposes..
  state.startMillis = millis();
  pins = new PINDEF();
  ESP_LOGI(TAG,"getDefaultPins");
  state.getDefaultPinDef(pins);
  ESP_LOGI(TAG,"wifi.createJsonDoc()");
  WifiController::createJsonDoc();

  ESP_LOGI(TAG,"state.setup");
  state.setup(pins);
  state.printInfo();

  ESP_LOGI(TAG,"Config::setup");
  Config::setup(pins);

  WifiController::getJDoc()->clear();
  // connect to wifi if necessary
  ESP_LOGI(TAG,"wifi.setup");
  WifiController::setup();
#ifdef IS_SLM
  ESP_LOGI(TAG,"IS_SLM");
  slm.setup();
#endif
#ifdef IS_LED
  ESP_LOGI(TAG,"IS_LED");
  LedController::setup(pins,false);
#endif

#ifdef IS_MOTOR
  ESP_LOGI(TAG,"IS_MOTOR");
#ifdef DEBUG_MOTOR
  motor.DEBUG = true;
#endif
  motor.setup(pins);
#endif

#if defined IS_PS4 || defined IS_PS3
  /*ESP_LOGI(TAG,"PsController Config::isFirstRun");
  if (Config::isFirstRun())
  {
    ESP_LOGI(TAG,"clear bt devices and restart");
    state.clearBlueetoothDevice();
    ESP.restart();
  }*/
#ifdef DEBUG_GAMEPAD
  ps_c.DEBUG = true;
#endif
  ps_c.start();
#endif

#ifdef IS_LASER
  laser.setup(pins);
#endif

#if defined IS_DAC || defined IS_DAC_FAKE
#if defined IS_DAC
  ESP_LOGI(TAG,"IS_DAC");
#endif
#if defined IS_DAC_FAKE
  ESP_LOGI(TAG,"IS_DAC_FAKE");
#endif
  dac.setup(pins);
#endif

#ifdef IS_ANALOG
  ESP_LOGI(TAG,"IS_ANALOG");
#ifdef DEBUG_ANALOG
  analog->DEBUG = true;
#endif
  analog.setup(pins);
#endif

#ifdef IS_DIGITAL
  digital.setup(pins);
#endif

#ifdef IS_READSENSOR
  ESP_LOGI(TAG,"IS_SENSOR");
  sensor.setup(pins);
#endif

#ifdef IS_PID
  ESP_LOGI(TAG,"IS_PID");
  pid.setup(pins);
#endif
#ifdef IS_SCANNER
  ESP_LOGI(TAG,"IS_SCANNER");
  scanner.setup(pins);
#endif

  ESP_LOGI(TAG,"End setup");
}

void loop()
{
  // for any timing-related purposes
  state.currentMillis = millis();
#ifdef IS_SERIAL
  serial.loop(WifiController::getJDoc());
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
  WifiController::handelMessages();
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
