#include "AS5311.h"
//https://raw.githubusercontent.com/IgorHeiden/ESP32InputPWMCalculator/main/esp32_mcpwm_pwm_meter.c

volatile uint32_t AS5311::_pos_edg_0 = 0;
volatile uint32_t AS5311::_pos_edg_1 = 0;
volatile uint32_t AS5311::_neg_edg_0 = 0;
volatile uint32_t AS5311::_edgeCounter = 0;
volatile bool AS5311::_time_2_print = false;
volatile AS5311::PWM_Params AS5311::_pwm = {0, 0};

int AS5311::_pwmPin = 0;
int AS5311::_interruptPin = 0;

AS5311::AS5311(int pwmPin, int interruptPin) {
    _pwmPin = pwmPin; // Set static members in the constructor
    _interruptPin = interruptPin;
}

void AS5311::begin() {
    pinMode(_pwmPin, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(_pwmPin), _Ext_PWM_ISR_handler, CHANGE);

    pinMode(_interruptPin, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(_interruptPin), _handleRisingEdge, CHANGE);

    hw_timer_t * timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &_print_adcpwm, false);
    timerAlarmWrite(timer, 100000, true);
    timerAlarmEnable(timer);
}

float AS5311::readPosition() {
    if (_time_2_print) {
        float currentPosition = (float)_pwm.duty_cycle/(float)_pwm.period*2000 + 2000*_edgeCounter;
        _time_2_print = false;
        return currentPosition;
    }
    return -1.0f;  // indicates no new data
}

void IRAM_ATTR AS5311::_handleRisingEdge() {
    float currentPosition = (float)_pwm.duty_cycle/(float)_pwm.period*2000 + 2000*_edgeCounter;
    if(currentPosition > 1000) {
        _edgeCounter++;
    } else {
        _edgeCounter--;
    }
}

void IRAM_ATTR AS5311::_Ext_PWM_ISR_handler() {
    uint32_t current_time = micros();
    if (digitalRead(_pwmPin) == HIGH) {
        _pos_edg_1 = current_time;
        _pwm.period = _pos_edg_1 - _pos_edg_0;
        _pos_edg_0 = _pos_edg_1;  
    } else {
        _neg_edg_0 = current_time;
        _pwm.duty_cycle = _neg_edg_0 - _pos_edg_0;
    }
}

void IRAM_ATTR AS5311::_print_adcpwm() {
    _time_2_print = true;
}
