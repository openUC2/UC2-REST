#pragma once
#include "../../config.h"
static const PROGMEM String state_act_endpoint = "/state_act";
static const PROGMEM String state_set_endpoint = "/state_set";
static const PROGMEM String state_get_endpoint = "/state_get";

#ifdef IS_LASER
static const PROGMEM String laser_act_endpoint = "/laser_act";
static const PROGMEM String laser_set_endpoint = "/laser_set";
static const PROGMEM String laser_get_endpoint = "/laser_get";
#endif

#ifdef IS_MOTOR
static const PROGMEM String motor_act_endpoint = "/motor_act";
static const PROGMEM String motor_set_endpoint = "/motor_set";
static const PROGMEM String motor_get_endpoint = "/motor_get";
#endif

#ifdef IS_DAC
static const PROGMEM String dac_act_endpoint = "/dac_act";
static const PROGMEM String dac_set_endpoint = "/dac_set";
static const PROGMEM String dac_get_endpoint = "/dac_get";
#endif

#ifdef IS_ANALOG
static const PROGMEM String analog_act_endpoint = "/analog_act";
static const PROGMEM String analog_set_endpoint = "/analog_set";
static const PROGMEM String analog_get_endpoint = "/analog_get";
#endif

#ifdef IS_DIGITAL
static const PROGMEM String digital_act_endpoint = "/digital_act";
static const PROGMEM String digital_set_endpoint = "/digital_set";
static const PROGMEM String digital_get_endpoint = "/digital_get";
#endif

#ifdef IS_LED
static const PROGMEM String ledarr_act_endpoint = "/ledarr_act";
static const PROGMEM String ledarr_set_endpoint = "/ledarr_set";
static const PROGMEM String ledarr_get_endpoint = "/ledarr_get";
#endif

#ifdef IS_SLM
static const PROGMEM String slm_act_endpoint = "/slm_act";
static const PROGMEM String slm_set_endpoint = "/slm_set";
static const PROGMEM String slm_get_endpoint = "/slm_get";
#endif

#ifdef IS_READSENSOR
static const PROGMEM String readsensor_act_endpoint = "/readsensor_act";
static const PROGMEM String readsensor_set_endpoint = "/readsensor_set";
static const PROGMEM String readsensor_get_endpoint = "/readsensor_get";
#endif

#ifdef IS_PID
static const PROGMEM String PID_act_endpoint = "/PID_act";
static const PROGMEM String PID_set_endpoint = "/PID_set";
static const PROGMEM String PID_get_endpoint = "/PID_get";
#endif

static const PROGMEM String config_act_endpoint = "/config_act";
static const PROGMEM String config_set_endpoint = "/config_set";
static const PROGMEM String config_get_endpoint = "/config_get";

static const PROGMEM String features_endpoint = "/features_get";
static const PROGMEM String identity_endpoint = "/identity";
static const PROGMEM String ota_endpoint = "/ota";
static const PROGMEM String update_endpoint = "/update";
static const PROGMEM String scanwifi_endpoint = "/wifi/scan";
static const PROGMEM String connectwifi_endpoint = "/wifi/connect";
static const PROGMEM String reset_nv_flash_endpoint = "/resetnv";

