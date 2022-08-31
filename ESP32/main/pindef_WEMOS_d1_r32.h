/*
  This map is for relatively common ESP32 boards replicating the form factor of Arduino UNO.
  This map allows use of such uno-compatible board with very popular
  "Protoneer Arduino CNC shield" and is based on its pinout.
  This makes perfect match for retrofiting older Arduino+GRBL based machines
  with 32b microcontroler capable of running grblHAL and providing few extra IO pins (eg. for modbus).
  These boards are sold under several names, for instance:
   + ESPDUINO-32  (USB type B)
   + Wemos D1 R32 (Micro USB)
*/


#define WEMOS_D1_R32_BOARD_NAME "ESPDUINO-32 Wemos D1 R32"

// timer definitions
#define WEMOS_D1_R32_STEP_TIMER_GROUP TIMER_GROUP_0
#define WEMOS_D1_R32_STEP_TIMER_INDEX TIMER_0

// Define step pulse output pins.
#define WEMOS_D1_R32_X_STEP_PIN          GPIO_NUM_26
#define WEMOS_D1_R32_Y_STEP_PIN          GPIO_NUM_25
#define WEMOS_D1_R32_Z_STEP_PIN          GPIO_NUM_17
#define WEMOS_D1_R32_A_STEP_PIN          GPIO_NUM_19

// Define step direction output pins. NOTE: All direction pins must be on the same port.
#define WEMOS_D1_R32_X_DIRECTION_PIN     GPIO_NUM_16
#define WEMOS_D1_R32_Y_DIRECTION_PIN     GPIO_NUM_27
#define WEMOS_D1_R32_Z_DIRECTION_PIN     GPIO_NUM_14
#define WEMOS_D1_R32_A_DIRECTION_PIN     GPIO_NUM_18

#define WEMOS_D1_R32_X_END_STOP          GPIO_NUM_13 // arduino 9
#define WEMOS_D1_R32_X_END_STOP          GPIO_NUM_5  // arduino 10
#define WEMOS_D1_R32_X_END_STOP          GPIO_NUM_23  // arduino 11


// Define stepper driver enable/disable output pin(s).
#define WEMOS_D1_R32_STEPPERS_ENABLE_PIN GPIO_NUM_12

// Define homing/hard limit switch input pins and limit interrupt vectors.
#define WEMOS_D1_R32_X_LIMIT_PIN         GPIO_NUM_13
#define WEMOS_D1_R32_Y_LIMIT_PIN         GPIO_NUM_5
#define WEMOS_D1_R32_Z_LIMIT_PIN         GPIO_NUM_23

// Define spindle enable and spindle direction output pins.
#define WEMOS_D1_R32_SPINDLE_ENABLE_PIN  GPIO_NUM_18
#define WEMOS_D1_R32_SPINDLEPWMPIN       GPIO_NUM_19

// Define flood enable output pin.
#define WEMOS_D1_R32_COOLANT_FLOOD_PIN   GPIO_NUM_32

// Define user-control CONTROLs (cycle start, reset, feed hold) input pins.
#define WEMOS_D1_R32_RESET_PIN           GPIO_NUM_2
#define WEMOS_D1_R32_FEED_HOLD_PIN       GPIO_NUM_4
#define WEMOS_D1_R32_CYCLE_START_PIN     GPIO_NUM_35

// Define probe switch input pin.
#define WEMOS_D1_R32_PROBE_PIN           GPIO_NUM_39

#define WEMOS_D1_R32_UART2_RX_PIN            GPIO_NUM_33
#define WEMOS_D1_R32_UART2_TX_PIN            GPIO_NUM_32
#define WEMOS_D1_R32_MODBUS_DIRECTION_PIN    GPIO_NUM_15
#define WEMOS_D1_R32_MODBUS_BAUD             19200


// Pin mapping when using SPI mode.
// With this mapping, SD card can be used both in SPI and 1-line SD mode.
// Note that a pull-up on CS line is required in SD mode.
#define WEMOS_D1_R32_PIN_NUM_MISO        GPIO_NUM_19
#define WEMOS_D1_R32_PIN_NUM_MOSI        GPIO_NUM_23
#define WEMOS_D1_R32_PIN_NUM_CLK         GPIO_NUM_18
#define WEMOS_D1_R32_PIN_NUM_CS          GPIO_NUM_5