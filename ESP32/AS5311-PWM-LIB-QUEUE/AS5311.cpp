#include "AS5311.h"
#include "freertos/queue.h"

volatile uint32_t AS5311::_pos_edg_0 = 0;
volatile uint32_t AS5311::_pos_edg_1 = 0;
volatile uint32_t AS5311::_neg_edg_0 = 0;
volatile AS5311::PWM_Params AS5311::_pwm = {0, 0, 0};
volatile int AS5311::_edgeCounter = 0;
volatile float AS5311::_position = 0.f;
volatile float AS5311::_offset = 0.f;
QueueHandle_t AS5311::dataQueue = nullptr; // Define the Queue handle globally.

int AS5311::_pwmPin = 0;
int AS5311::_interruptPin = 0;
bool AS5311::_is_isr_service_installed = false;

AS5311::AS5311(int pwmPin, int interruptPin, bool is_isr_service_installed = false)
{
  AS5311::_pwmPin = pwmPin;
  _interruptPin = interruptPin;
  _is_isr_service_installed = is_isr_service_installed;
}

void AS5311::begin()
{
  if (!_is_isr_service_installed)
  {
    gpio_install_isr_service(0);
  }
  gpio_config_t io_conf_pwm = {
      .pin_bit_mask = (1ULL << _pwmPin),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_ENABLE,
      .intr_type = GPIO_INTR_ANYEDGE, // To handle CHANGE interrupt type
  };
  gpio_config(&io_conf_pwm);

  gpio_config_t io_conf_interrupt = {
      .pin_bit_mask = (1ULL << _interruptPin),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_ENABLE,
      .intr_type = GPIO_INTR_POSEDGE, // To handle RISING interrupt type
  };
  gpio_config(&io_conf_interrupt);


  gpio_isr_handler_add((gpio_num_t)_pwmPin, _Ext_PWM_ISR_handler, (void *)_pwmPin);
  gpio_isr_handler_add((gpio_num_t)_interruptPin, _handleRisingEdge, (void *)_interruptPin);

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

float AS5311::readPWM()
{
  return _position;
}

int AS5311::readEdgeCounter()
{
  return _edgeCounter;
}

float AS5311::readPosition()
{
  return 2000. * (_edgeCounter + _position) + _offset;
}

void AS5311::setOffset(float offset)
{
  _offset = offset;
}

float AS5311::getOffset()
{
  return _offset;
}

void IRAM_ATTR AS5311::_handleRisingEdge(void* arg)
{
  log_i("Rising edge detected on pin %d", (uint32_t)arg);
  PWM_Params localData;
  localData.period = 0;
  localData.duty_cycle = 0;
  localData.edgeCounter = 1;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(dataQueue, &localData, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR();
  }
}

void IRAM_ATTR AS5311::_Ext_PWM_ISR_handler(void* arg)
{
  uint32_t current_time = micros();
  PWM_Params localData;
  localData.period = 0;
  localData.duty_cycle = 0;
  localData.edgeCounter = 0;

  if (digitalRead(_pwmPin) == HIGH)
  {
    _pos_edg_1 = current_time;
    localData.period = _pos_edg_1 - _pos_edg_0;
    _pos_edg_0 = _pos_edg_1;
  }
  else
  {
    _neg_edg_0 = current_time;
    localData.duty_cycle = _neg_edg_0 - _pos_edg_0;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(dataQueue, &localData, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR();
  }
}

void AS5311::handleDataTask(void *parameter)
{
  PWM_Params receivedData;
  PWM_Params localData;
  localData = {0, 0, 0};
  float position = 0.f;
  for (;;)
  {
    if (xQueueReceive(dataQueue, &receivedData, portMAX_DELAY))
    {
      if (receivedData.edgeCounter != 0)
      {
        if (_position > .5)
        {
          _edgeCounter = _edgeCounter + 1;
        }
        else
        {
          _edgeCounter = _edgeCounter - 1;
        }
        // Serial.print("Edge Counter: ");
        // Serial.println(_edgeCounter);
      }

      if (receivedData.period != 0)
      {
        // Serial.print("Period: ");
        // Serial.println(receivedData.period);
        localData.period = receivedData.period;
      }
      if (receivedData.duty_cycle != 0)
      {
        // Serial.print("Duty Cycle: ");
        // Serial.println(receivedData.duty_cycle);
        localData.duty_cycle = receivedData.duty_cycle;
      }

      // Calculate position
      if (localData.period != 0 and localData.duty_cycle <= localData.period)
      {
        // At the 0/1 change the duty cycle gets noisy, so we need to average it over multiple senses
        _position = calculateRollingAverage((float)localData.duty_cycle / (float)localData.period);
      }
    }
  }
}

float AS5311::calculateRollingAverage(float newVal)
{
  static float values[3] = {0.0, 0.0, 0.0};
  static int insertIndex = 0;
  static float sum = 0.0;

  sum -= values[insertIndex];
  values[insertIndex] = newVal;
  sum += newVal;

  insertIndex = (insertIndex + 1) % 3;

  return sum / 3.0;
}
