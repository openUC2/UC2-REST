#include "AS5311.h"
#include "freertos/queue.h"

volatile uint32_t AS5311::_pos_edg_0 = 0;
volatile uint32_t AS5311::_pos_edg_1 = 0;
volatile uint32_t AS5311::_neg_edg_0 = 0;
volatile AS5311::PWM_Params AS5311::_pwm = {0, 0, 0};
volatile int AS5311::_edgeCounter = 0;
QueueHandle_t AS5311::dataQueue = nullptr;  // Define the Queue handle globally.

AS5311::AS5311(int pwmPin, int interruptPin) {
  _pwmPin = pwmPin; // Set static members in the constructor
  _interruptPin = interruptPin;
}

void AS5311::begin() {
  pinMode(_pwmPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(_pwmPin), _Ext_PWM_ISR_handler, CHANGE);

  pinMode(_interruptPin, INPUT_PULLDOWN);
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

float AS5311::readPosition() {
  // This method may need adjustment according to your new data handling logic.
  return -1.;
}

int AS5311::readEdgeCounter() {
  return _edgeCounter;
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

void IRAM_ATTR AS5311::_Ext_PWM_ISR_handler() {
  uint32_t current_time = micros();
  PWM_Params localData;
  localData.period = 0;
  localData.duty_cycle = 0;
  localData.edgeCounter = 1; 

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
  for (;;) {
    if (xQueueReceive(dataQueue, &receivedData, portMAX_DELAY)) {
      Serial.println(receivedData.period);
      Serial.println(receivedData.edgeCounter);
      //_edgeCounter = receivedData.edgeCounter;
      //Serial.println(receivedData);
      // You might process the received data here, or update other variables accordingly.
    }
  }
}
