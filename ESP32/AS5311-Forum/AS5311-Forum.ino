#include <Arduino.h>

const int PWM_PIN = GPIO_NUM_26;

volatile uint32_t pos_edg_0 = 0;
volatile uint32_t pos_edg_1 = 0;
volatile uint32_t neg_edg_0 = 0;
volatile uint8_t time_2_print = 0;

const int INTERRUPT_PIN = 27;  // New interrupt pin
volatile uint32_t edgeCounter = 0;  // Counter for rising edges
float currentPosition = 0;


void IRAM_ATTR handleRisingEdge() {  // ISR for the new interrupt
    if(currentPosition > 1000){ // 
      edgeCounter++;
    }
    else{
      edgeCounter--;
    }
}


typedef struct {
    uint32_t period;
    uint32_t duty_cycle;
} PWM_Params;

volatile PWM_Params pwm = {0, 0};
hw_timer_t * timer = NULL;

void IRAM_ATTR Ext_PWM_ISR_handler() {
    uint32_t current_time = micros();

    if (digitalRead(PWM_PIN) == HIGH) {  // Rising edge
        pos_edg_1 = current_time;
        pwm.period = pos_edg_1 - pos_edg_0;
        pos_edg_0 = pos_edg_1;  
    } else {  // Falling edge
        neg_edg_0 = current_time;
        pwm.duty_cycle = neg_edg_0 - pos_edg_0;
    }
}

void IRAM_ATTR print_adcpwm() {
    time_2_print = 1;
}

void setup() {
    Serial.begin(115200);

    pinMode(PWM_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(PWM_PIN), Ext_PWM_ISR_handler, CHANGE);

    // Setup for the new interrupt
    pinMode(INTERRUPT_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleRisingEdge, CHANGE);

    // Setup timer
    timer = timerBegin(0, 80, true);  // Use timer 0, with a prescaler of 80
    timerAttachInterrupt(timer, &print_adcpwm, false);
    timerAlarmWrite(timer, 100000, true);  // Set the timer to fire every 3 seconds
    timerAlarmEnable(timer);
}

void loop() {
    if (time_2_print) {
        currentPosition = (float)pwm.duty_cycle/(float)pwm.period*2000+2000*edgeCounter;
        Serial.println(currentPosition);
        time_2_print = 0;
        Serial.printf("Rising Edge Counter: %u\n", edgeCounter);  // Print the new counter
        
    }
    delay(10);  // Small delay to prevent tight loop.
}
