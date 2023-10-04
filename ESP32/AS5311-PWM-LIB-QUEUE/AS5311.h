#ifndef AS5311_H
#define AS5311_H

#include <Arduino.h>

class AS5311 {
  public:
    AS5311(int pwmPin, int interruptPin);
    
    void begin();
    float readPosition();
    int readEdgeCounter();
  private:
    static int _pwmPin, _interruptPin;
    static bool writing_counter;
    static volatile uint32_t _pos_edg_0, _pos_edg_1, _neg_edg_0;
    static volatile int _edgeCounter;
    static volatile float _position;
    static volatile bool _time_2_print;
    static QueueHandle_t dataQueue;  // Define the Queue handle globally.
    static void handleDataTask(void *parameter);
    
    typedef struct {
      uint32_t period;
      uint32_t duty_cycle;
      int edgeCounter; 
    } PWM_Params;

    static volatile PWM_Params _pwm;

    static void IRAM_ATTR _handleRisingEdge();
    static void IRAM_ATTR _Ext_PWM_ISR_handler();
    static void IRAM_ATTR _print_adcpwm();
};

#endif
