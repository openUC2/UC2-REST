// DAC-specific parameters
#include "DAC_Module.h"

dac_channel_t dac_channel = DAC_CHANNEL_1;

uint32_t clk_div = 0;
uint32_t scale = 0;
uint32_t invert = 2;
uint32_t phase = 0;

boolean dac_is_running = false;
