#pragma once
#include "../../config.h"
static const char* state_act_endpoint = "/state_act";
static const char* state_set_endpoint = "/state_set";
static const char* state_get_endpoint = "/state_get";

#ifdef IS_LASER
static const char* laser_act_endpoint = "/laser_act";
static const char* laser_set_endpoint = "/laser_set";
static const char* laser_get_endpoint = "/laser_get";
#endif

#ifdef IS_MOTOR
static const char* motor_act_endpoint = "/motor_act";
static const char* motor_set_endpoint = "/motor_set";
static const char* motor_get_endpoint = "/motor_get";
#endif

#ifdef IS_DAC
static const char* dac_act_endpoint = "/dac_act";
static const char* dac_set_endpoint = "/dac_set";
static const char* dac_get_endpoint = "/dac_get";
#endif

#ifdef IS_ANALOG
static const char* analog_act_endpoint = "/analog_act";
static const char* analog_set_endpoint = "/analog_set";
static const char* analog_get_endpoint = "/analog_get";
#endif

#ifdef IS_DIGITAL
static const char* digital_act_endpoint = "/digital_act";
static const char* digital_set_endpoint = "/digital_set";
static const char* digital_get_endpoint = "/digital_get";
#endif

static const char* ledarr_act_endpoint = "/ledarr_act";
static const char* ledarr_set_endpoint = "/ledarr_set";
static const char* ledarr_get_endpoint = "/ledarr_get";

#ifdef IS_SLM
static const char* slm_act_endpoint = "/slm_act";
static const char* slm_set_endpoint = "/slm_set";
static const char* slm_get_endpoint = "/slm_get";
#endif

#ifdef IS_READSENSOR
static const char* readsensor_act_endpoint = "/readsensor_act";
static const char* readsensor_set_endpoint = "/readsensor_set";
static const char* readsensor_get_endpoint = "/readsensor_get";
#endif

#ifdef IS_PID
static const char* PID_act_endpoint = "/PID_act";
static const char* PID_set_endpoint = "/PID_set";
static const char* PID_get_endpoint = "/PID_get";
#endif

static const char* config_act_endpoint = "/config_act";
static const char* config_set_endpoint = "/config_set";
static const char* config_get_endpoint = "/config_get";

