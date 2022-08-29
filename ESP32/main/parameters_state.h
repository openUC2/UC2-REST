const char*  identifier_name = "UC2_Feather";
const char*  identifier_id = "V1.2";
const char*  identifier_date = __DATE__ "" __TIME__;
const char*  identifier_author = "BD";
String IDENTIFIER_NAME = "";

// timing variables
unsigned long startMillis;
unsigned long currentMillis;
bool isBusy = false;


/*
Endpoint definitions
*/

const char* state_act_endpoint = "/state_act";
const char* state_set_endpoint = "/state_set";
const char* state_get_endpoint = "/state_get";

const char* laser_act_endpoint = "/laser_act";
const char* laser_set_endpoint = "/laser_set";
const char* laser_get_endpoint = "/laser_get";

const char* motor_act_endpoint = "/motor_act";
const char* motor_set_endpoint = "/motor_set";
const char* motor_get_endpoint = "/motor_get";

const char* dac_act_endpoint = "/dac_act";
const char* dac_set_endpoint = "/dac_set";
const char* dac_get_endpoint = "/dac_get";

const char* analog_act_endpoint = "/analog_act";
const char* analog_set_endpoint = "/analog_set";
const char* analog_get_endpoint = "/analog_get";

const char* digital_act_endpoint = "/digital_act";
const char* digital_set_endpoint = "/digital_set";
const char* digital_get_endpoint = "/digital_get";

const char* ledarr_act_endpoint = "/ledarr_act";
const char* ledarr_set_endpoint = "/ledarr_set";
const char* ledarr_get_endpoint = "/ledarr_get";

const char* slm_act_endpoint = "/slm_act";
const char* slm_set_endpoint = "/slm_set";
const char* slm_get_endpoint = "/slm_get";

const char* readsensor_act_endpoint = "/readsensor_act";
const char* readsensor_set_endpoint = "/readsensor_set";
const char* readsensor_get_endpoint = "/readsensor_get";

const char* PID_act_endpoint = "/PID_act";
const char* PID_set_endpoint = "/PID_set";
const char* PID_get_endpoint = "/PID_get";

