int LASER_val_1 = 0;
int LASER_val_2 = 0;
int LASER_val_3 = 0;

// PWM Stuff - ESP only
int pwm_resolution = 10;
long pwm_frequency = 5000;//19000; //12000
long pwm_max = (int)pow(2, pwm_resolution);

int PWM_CHANNEL_LASER_1 = 0;
int PWM_CHANNEL_LASER_2 = 1;
int PWM_CHANNEL_LASER_3 = 2;

// temperature dependent despeckeling?
int LASER_despeckle_1 = 0;
int LASER_despeckle_2 = 0;
int LASER_despeckle_3 = 0;

int LASER_despeckle_period_1 = 20;
int LASER_despeckle_period_2 = 20;
int LASER_despeckle_period_3 = 20;
