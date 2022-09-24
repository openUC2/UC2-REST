#pragma once

#include "../../config.h"
#include <ArduinoJson.h>
#ifdef IS_MOTOR
#include "../motor/FocusMotor.h"
#endif
#ifdef IS_LED
#include "../led/LedController.h"
#endif
#ifdef IS_LASER
#include "../laser/LaserController.h"
#endif
#ifdef IS_ANALOG
#include "../analog/AnalogController.h"
#endif
#include "../state/State.h"
#ifdef IS_SCANNER
#include "../scanner/ScannerController.h"
#endif
#ifdef IS_PID
#include "../pid/PidController.h"
#endif
#ifdef IS_DIGITAL
#include "../digital/DigitalController.h"
#endif
#ifdef IS_READSENSOR
#include "../sensor/SensorController.h"
#endif
#if defined IS_DAC || defined IS_DAC_FAKE
#include "../dac/DacController.h"
#endif
#ifdef IS_SLM
#include "../slm/SlmController.h"
#endif
#include "../config/ConfigController.h"
#include "../wifi/Endpoints.h"

class SerialProcess
{
private:
    void jsonProcessor(String task,DynamicJsonDocument * jsonDocument);
    void tableProcessor(DynamicJsonDocument * jsonDocument);
    /* data */
public:
    SerialProcess(/* args */);
    ~SerialProcess();
    void loop(DynamicJsonDocument * jsonDocument);
};

static SerialProcess serial;
