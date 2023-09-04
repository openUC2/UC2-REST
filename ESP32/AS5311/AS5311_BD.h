#ifndef AS5311_h
#define AS5311_h

#include "Arduino.h"

class AS5311
{
  public:
    AS5311(uint16_t DataPin, uint16_t ClockPin, uint16_t ChipSelectPin);
    uint32_t encoder_position(void);
    uint32_t encoder_value(void);
    uint32_t encoder_error(void);
    struct err_struct{
    bool DECn;
  bool INCn;
  bool OCF;
  bool COF;
  bool LIN; } err_value;
  private:
    uint32_t read_chip(void);
    const uint16_t _clock;        // clock pin: output from arduino to as5311
    const uint16_t _cs;           // chip select: output
    const uint16_t _data;         // data pin: input
    
};

#endif
