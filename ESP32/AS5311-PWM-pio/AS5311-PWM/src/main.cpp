#include "Arduino.h"
#include "driver/mcpwm.h"
#include "driver/gpio.h"

#define GPIO_CAP0_IN   GPIO_NUM_26
#define GPIO_CAP1_IN   GPIO_NUM_27
#define GPIO_TRIGGER   GPIO_NUM_14  // New GPIO pin for triggering the counter

struct CaptureData {
    uint32_t time_rising_edge;
    uint32_t time_falling_edge;
    uint32_t period;
    bool last_was_rising;
};

CaptureData cap_data[2];
volatile float latest_duty_cycle[2] = {0.0f, 0.0f};  // Updated by the MCPWM capture ISR
volatile int counter = 0;

void IRAM_ATTR capture_handler(void *arg) {
    uint32_t cap_unit = (uint32_t)arg;
    uint32_t curr_time = mcpwm_capture_signal_get_value(MCPWM_UNIT_0, (mcpwm_capture_signal_t)cap_unit);
    
    if (mcpwm_capture_signal_get_edge(MCPWM_UNIT_0, (mcpwm_capture_signal_t)cap_unit) == MCPWM_POS_EDGE) {
        if (cap_data[cap_unit].last_was_rising) {
            cap_data[cap_unit].period = curr_time - cap_data[cap_unit].time_rising_edge;
        }
        cap_data[cap_unit].time_rising_edge = curr_time;
        cap_data[cap_unit].last_was_rising = true;
    } else {
        cap_data[cap_unit].time_falling_edge = curr_time;
        cap_data[cap_unit].last_was_rising = false;
    }

    if (cap_data[cap_unit].period != 0) {
        uint32_t high_time = cap_data[cap_unit].time_falling_edge - cap_data[cap_unit].time_rising_edge;
        latest_duty_cycle[cap_unit] = ((float)high_time / (float)cap_data[cap_unit].period) * 100.0f;
        Serial.printf("CAP%d Duty cycle: %0.2f%%\n", cap_unit, latest_duty_cycle[cap_unit]);
    }
}

void IRAM_ATTR trigger_handler(void* arg) {
    for (int i = 0; i < 2; i++) {
        if (latest_duty_cycle[i] > 50.0f) {
            counter++;
        } else {
            counter--;
        }
    }
    Serial.printf("Counter (triggered): %d\n", counter);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Testing MCPWM capture on two pins...");

    // Initialize MCPWM capture
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_0, GPIO_CAP0_IN);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_1, GPIO_CAP1_IN);

    mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, MCPWM_BOTH_EDGE, 0);
    mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP1, MCPWM_BOTH_EDGE, 0);

    intr_handle_t cap0_intr_handle = NULL;
    intr_handle_t cap1_intr_handle = NULL;

    mcpwm_isr_register(MCPWM_UNIT_0, capture_handler, (void*)MCPWM_SELECT_CAP0, ESP_INTR_FLAG_IRAM, &cap0_intr_handle);
    mcpwm_isr_register(MCPWM_UNIT_0, capture_handler, (void*)MCPWM_SELECT_CAP1, ESP_INTR_FLAG_IRAM, &cap1_intr_handle);

    // Initialize the GPIO_TRIGGER pin for external interrupt
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE; // Trigger on rising edge
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_TRIGGER);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    // Register ISR for the trigger pin
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(GPIO_TRIGGER, trigger_handler, NULL);

    // Clear capture data
    memset(cap_data, 0, sizeof(cap_data));
}

void loop() {
    delay(1000);
}
