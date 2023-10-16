#include "AS5311.h"
#include "freertos/queue.h"

volatile uint32_t AS5311::_pos_edg_0 = 0;
volatile uint32_t AS5311::_pos_edg_1 = 0;
volatile uint32_t AS5311::_neg_edg_0 = 0;
volatile AS5311::PWM_Params AS5311::_pwm = {0, 0, 0};
volatile int AS5311::_edgeCounter = 0;
volatile float AS5311::_position = 0.f;
volatile float AS5311::_offset = 0.f;
QueueHandle_t AS5311::dataQueue = nullptr;  // Define the Queue handle globally.

int AS5311::_pwmPin = 0;
int AS5311::_interruptPin = 0;

AS5311::AS5311(int pwmPin, int interruptPin) {
  AS5311::_pwmPin = pwmPin; 
  _interruptPin = interruptPin;
}

void AS5311::begin() {
  pinMode(_pwmPin, INPUT_PULLDOWN);
  pinMode(_interruptPin, INPUT_PULLDOWN);
  
  attachInterrupt(digitalPinToInterrupt(_pwmPin), _Ext_PWM_ISR_handler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(_interruptPin), _handleRisingEdge, RISING); 

  // Create a queue to handle PWM and EdgeCounter data in a safe context
  dataQueue = xQueueCreate(10, sizeof(PWM_Params));

  // Task creation should be added here
  xTaskCreate(
    handleDataTask,     /* Task function. */
    "HandleData",       /* String with name of task. */
    10000,              /* Stack size in bytes. */
    NULL,               /* Parameter passed as input to the task */
    1,                  /* Priority at which the task is created. */
    NULL);              /* Task handle. */
}

float AS5311::readPWM() {
  return _position;
}

int AS5311::readEdgeCounter() {
  return _edgeCounter;
}

float AS5311::readPosition() {
  return 2000.*(_edgeCounter+_position)+_offset;
}

void AS5311::setOffset(float offset) {
  _offset = offset;
}

float AS5311::getOffset() {
  return _offset;
}

void IRAM_ATTR AS5311::_handleRisingEdge() {
  PWM_Params localData;
  localData.period = 0;
  localData.duty_cycle = 0;
  localData.edgeCounter = 1; 
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(dataQueue, &localData, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

void IRAM_ATTR  AS5311::_Ext_PWM_ISR_handler() {
  uint32_t current_time = micros();
  PWM_Params localData;
  localData.period = 0;
  localData.duty_cycle = 0;
  localData.edgeCounter = 0; 

  if (digitalRead(_pwmPin) == HIGH) {
    _pos_edg_1 = current_time;
    localData.period = _pos_edg_1 - _pos_edg_0;
    _pos_edg_0 = _pos_edg_1;
  } else {
    _neg_edg_0 = current_time;
    localData.duty_cycle = _neg_edg_0 - _pos_edg_0;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(dataQueue, &localData, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

void AS5311::handleDataTask(void *parameter) {
  PWM_Params receivedData;
  PWM_Params localData;
  localData = {0, 0, 0};
  float position = 0.f;
  for (;;) {
    if (xQueueReceive(dataQueue, &receivedData, portMAX_DELAY)) {
      if (receivedData.edgeCounter != 0) {
        if (_position>.5){
          _edgeCounter = _edgeCounter +1;
        }
        else{
          _edgeCounter = _edgeCounter -1;
        }
        //Serial.print("Edge Counter: ");
        //Serial.println(_edgeCounter);
      }

      if(receivedData.period != 0) {
        //Serial.print("Period: ");
        //Serial.println(receivedData.period);
        localData.period = receivedData.period;
      }
      if (receivedData.duty_cycle != 0) {
        //Serial.print("Duty Cycle: ");
        //Serial.println(receivedData.duty_cycle);
        localData.duty_cycle = receivedData.duty_cycle;
      }

      // Calculate position
      if (localData.period !=0 and localData.duty_cycle <= localData.period){
        // At the 0/1 change the duty cycle gets noisy, so we need to average it over multiple senses
       _position = calculateRollingAverage((float) localData.duty_cycle / (float) localData.period);
      }
       
    }
  }
}

float AS5311::calculateRollingAverage(float newVal) {
  static float values[3] = {0.0, 0.0, 0.0};
  static int insertIndex = 0;  
  static float sum = 0.0;
    
  sum -= values[insertIndex];
  values[insertIndex] = newVal;
  sum += newVal;
    
  insertIndex = (insertIndex + 1) % 3;
    
  return sum / 3.0;
}
