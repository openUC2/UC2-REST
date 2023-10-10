#include "AS5311AB.h"
#include "driver/gpio.h"

volatile int AS5311AB::encoder_pos = 0;
QueueHandle_t AS5311AB::encoderQueue = NULL;
uint8_t AS5311AB::_pinA = 0;
uint8_t  AS5311AB::_pinB = 0;
bool AS5311AB::_is_isr_service_installed = false;

AS5311AB::AS5311AB(uint8_t pinA, uint8_t pinB, bool is_isr_service_installed = false)
    {
    _pinA = pinA;
    _pinB = pinB;
    _is_isr_service_installed = is_isr_service_installed;
}

void AS5311AB::begin()
{

    // Install the ISR service only once
    if (!_is_isr_service_installed)
    {
        gpio_install_isr_service(0);
    }

    log_i("Attaching pins %d and %d", _pinA, _pinB);

    gpio_config_t io_conf_a = {
        .pin_bit_mask = (1ULL << _pinA),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE, // To handle CHANGE interrupt type
    };
    gpio_config(&io_conf_a);

    gpio_config_t io_conf_b = {
        .pin_bit_mask = (1ULL << _pinB),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE, // To handle RISING interrupt type
    };
    gpio_config(&io_conf_b);

    // Attach ISRs  
    gpio_isr_handler_add((gpio_num_t)_pinA, handleAChange, (void *)_pinA);
    gpio_isr_handler_add((gpio_num_t)_pinB, handleBChange, (void *)_pinB);
    //gpio_isr_handler_add((gpio_num_t)_pinA, handleAChange, (void *)this);
    //gpio_isr_handler_add((gpio_num_t)_pinB, handleBChange, (void *)this);

    encoderQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(processEncoderData, "ProcessEncoder", 2048, NULL, 2, NULL);
}

int AS5311AB::getPosition()
{
    int pos;
    xQueueReceive(encoderQueue, &pos, 0); // Non-blocking read
    return pos;
}

void IRAM_ATTR AS5311AB::handleAChange(void *arg)
{
    //AS5311AB *self = (AS5311AB *)arg;
    //uint8_t pinA = self->_pinA;
    //uint8_t pinB = self->_pinB;
    log_i("A changed");
    if (digitalRead(_pinA) == digitalRead(_pinB))
    {
        encoder_pos++;
    }
    else
    {
        encoder_pos--;
    }
    xQueueSendFromISR(encoderQueue, (void *)&encoder_pos, NULL);
}

void IRAM_ATTR AS5311AB::handleBChange(void *arg)
{
    //AS5311AB *self = (AS5311AB *)arg;
    //uint8_t pinA = self->_pinA;
    //uint8_t pinB = self->_pinB;
    log_i("B changed with pin A: %d and pin B: %d", pinA, pinB);
    if (digitalRead(_pinA) != digitalRead(_pinB))
    {
        encoder_pos++;
    }
    else
    {
        encoder_pos--;
    }
    xQueueSendFromISR(encoderQueue, (void *)&encoder_pos, NULL);
}

void AS5311AB::processEncoderData(void *parameter)
{
    int position;
    for (;;)
    {
        if (xQueueReceive(encoderQueue, &position, portMAX_DELAY))
        {
            log_i("Position: %d", position);
            // Additional data processing can be performed here
            // Note: Be cautious about using Serial or other non-ISR safe functions inside a task
        }
    }
}
