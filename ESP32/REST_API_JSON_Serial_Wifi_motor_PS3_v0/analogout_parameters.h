int analogout_PIN_1 = 22;
int analogout_PIN_2 = 23;
int analogout_PIN_3 = 0;

#ifndef IS_LASER
// PWM Stuff - ESP only
int pwm_resolution = 15;
int pwm_frequency = 800000;//19000; //12000
int pwm_max = (int)pow(2, pwm_resolution);
#endif

int analogout_val_1 = 0;
int analogout_val_2 = 0;
int analogout_val_3 = 0;
int analogout_VIBRATE = 0;

int analogout_SOFI_1 = 0;
int analogout_SOFI_2 = 0;

int PWM_CHANNEL_analogout_1 = 4;
int PWM_CHANNEL_analogout_2 = 5;
int PWM_CHANNEL_analogout_3 = 6;
