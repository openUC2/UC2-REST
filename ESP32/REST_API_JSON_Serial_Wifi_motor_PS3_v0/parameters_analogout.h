
#include "esp_err.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/periph_ctrl.h"
#include "soc/ledc_reg.h"

#define J1772_LEDC_TIMER       LEDC_TIMER_0
#define J1772_LEDC_CHANNEL     LEDC_CHANNEL_0
#define J1772_LEDC_TIMER_RES   LEDC_TIMER_9_BIT
#define J1772_DUTY_MAX         ((1 << LEDC_TIMER_9_BIT) -1 )
#define J1772_PWM_FREQUENCY_HZ 1000
#define J1772_LEDC_SPEEDMODE   LEDC_HIGH_SPEED_MODE

// PWM Stuff - ESP only
int pwm_resolution = 15;
int pwm_frequency = 80000;//19000; //12000
int pwm_max = (int)pow(2, pwm_resolution);

int analogout_val_1 = 0;
int analogout_val_2 = 0;
int analogout_val_3 = 0;
int analogout_VIBRATE = 0;

int analogout_SOFI_1 = 0;
int analogout_SOFI_2 = 0;

int PWM_CHANNEL_analogout_1 = 4;
int PWM_CHANNEL_analogout_2 = 5;
int PWM_CHANNEL_analogout_3 = 6;
