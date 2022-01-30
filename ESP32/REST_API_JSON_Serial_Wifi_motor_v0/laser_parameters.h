int LASER_PIN_1 = 18;// 22;
int LASER_PIN_2 = 19;
int LASER_PIN_3 = 21;

int LASER_val_1 = 0;
int LASER_val_2 = 0;
int LASER_val_3 = 0;

// PWM Stuff
int pwm_resolution = 15;
int pwm_frequency = 800000;//19000; //12000
int pwm_max = (int)pow(2, pwm_resolution);

int PWM_CHANNEL_LASER_3 = 2;
int PWM_CHANNEL_LASER_2 = 1;
int PWM_CHANNEL_LASER_1 = 0;
