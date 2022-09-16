#pragma once
#include "../../config.h"
static const String state_act_endpoint = F("/state_act");
static const String state_set_endpoint = F("/state_set");
static const String state_get_endpoint = F("/state_get");

#ifdef IS_LASER
static const String laser_act_endpoint = F("/laser_act");
static const String laser_set_endpoint = F("/laser_set");
static const String laser_get_endpoint = F("/laser_get");
#endif

#ifdef IS_MOTOR
static const String motor_act_endpoint = F("/motor_act");
static const String motor_set_endpoint = F("/motor_set");
static const String motor_get_endpoint = F("/motor_get");
#endif

#ifdef IS_DAC
static const String dac_act_endpoint = F("/dac_act");
static const String dac_set_endpoint = F("/dac_set");
static const String dac_get_endpoint = F("/dac_get");
#endif

#ifdef IS_ANALOG
static const String analog_act_endpoint = F("/analog_act");
static const String analog_set_endpoint = F("/analog_set");
static const String analog_get_endpoint = F("/analog_get");
#endif

#ifdef IS_DIGITAL
static const String digital_act_endpoint = F("/digital_act");
static const String digital_set_endpoint = F("/digital_set");
static const String digital_get_endpoint = F("/digital_get");
#endif

#ifdef IS_LED
static const String ledarr_act_endpoint = F("/ledarr_act");
static const String ledarr_set_endpoint = F("/ledarr_set");
static const String ledarr_get_endpoint = F("/ledarr_get");
#endif

#ifdef IS_SLM
static const String slm_act_endpoint = F("/slm_act");
static const String slm_set_endpoint = F("/slm_set");
static const String slm_get_endpoint = F("/slm_get");
#endif

#ifdef IS_READSENSOR
static const String readsensor_act_endpoint = F("/readsensor_act");
static const String readsensor_set_endpoint = F("/readsensor_set");
static const String readsensor_get_endpoint = F("/readsensor_get");
#endif

#ifdef IS_PID
static const String PID_act_endpoint = F("/PID_act");
static const String PID_set_endpoint = F("/PID_set");
static const String PID_get_endpoint = F("/PID_get");
#endif

static const String config_act_endpoint = F("/config_act");
static const String config_set_endpoint = F("/config_set");
static const String config_get_endpoint = F("/config_get");

static const PROGMEM String features_endpoint = "/features_get";
static const PROGMEM String identity_endpoint = "/identity";
static const PROGMEM String ota_endpoint = "/ota";
static const PROGMEM String update_endpoint = "/update";
static const PROGMEM String scanwifi_endpoint = "/wifi/scan";

