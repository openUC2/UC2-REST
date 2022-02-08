int LASER_val_1 = 0;
int LASER_val_2 = 0;
int LASER_val_3 = 0;

// PWM Stuff - ESP only
int pwm_resolution = 15;
int pwm_frequency = 800000;//19000; //12000
int pwm_max = (int)pow(2, pwm_resolution);

int PWM_CHANNEL_LASER_1 = 0;
int PWM_CHANNEL_LASER_2 = 1;
int PWM_CHANNEL_LASER_3 = 2;
