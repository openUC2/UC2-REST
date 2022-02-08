# UC2 REST API

This is the playground to start development of using UC2 modules using the HTTP REST API. 

## ESP32

This folder contains all the code for handling actuators and sensors through HTTP requests. 

## PYTHON

This provides a simple client to control all kinds of actions.


# Installation

### Install Arduino IDE

- Download the Arduino IDE 1.8.1 from [here](https://www.arduino.cc/en/software/OldSoftwareReleases)
- Install it

### Install Serial driver

In case you use a chinese derivate Arduino or an ESP32 board, you most likely need to install the ***CH340*** serial driver. Please have a look [here](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all)

### Get the right Boards

If you want an ESP32 board, please add the following sources to the preferences (Boards). More information can be found [here](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)

```
https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

### Install the required libraries

Go to Tools -> Manage Libraries and add the following libraries (More information [here](https://arduinogetstarted.com/faq/how-to-install-library-on-arduino-ide):

```
ArduinoJson
StepperMotor
```

### Compile and Upload Arduino Firmware

- Download this repository following this [link](https://github.com/openUC2/UC2-REST/archive/refs/heads/master.zip)
- Go to the folder that contains the file `REST_API_JSON_Serial_Wifi_motor_PS3_v0.ino` in [.ESP32/REST_API_JSON_Serial_Wifi_motor_PS3_v0](https://github.com/openUC2/UC2-REST/tree/master/ESP32/REST_API_JSON_Serial_Wifi_motor_PS3_v0)
- Select the board you want to install it to (e.g. Arduino or ESP32 from the Boardmanager) 
- *Optional* adapt some settings (e.g. adding modules, selecting the communication channel like Wifi / Serial) by commenting/outcommenting the following lines
```
 // CASES:
// 1 Arduino -> Serial only
// 2 ESP32 -> Serial only
// 3 ESP32 -> Wifi only
// 4 ESP32 -> Wifi + Serial ?

// load configuration
#define ARDUINO_SERIAL
//#define ESP32_SERIAL
//#define ESP32_WIFI
//#define ESP32_SERIAL_WIFI

....


// load modules
# ifdef IS_ESP32
#define IS_DAC // ESP32-only
#define IS_PS3 // ESP32-only
#define IS_ANALOGOUT// ESP32-only
#endif
#define IS_LASER
#define IS_MOTOR

```
- *Optional*: Adapt some pin settings in thhe file `pindef.h`


### Test the code using the Arduino Serial

- Open the Arduino Serial (more information [here](https://starthardware.org/arduino-serial-print/)
- Set the Baudrate to `115200` and enter some `Json` commands to manipulate the actuators
  - Identify the Board: `{"task": "/state_get"}`
  - Turn on the laser: `{"task": "/laser_act", "LASERid":1, "LASERval":2}`
  - Move the motor: `{"task": "/motor_act", "axis":1, "speed":1000, "position":1000, "isabsolute":1, "isblocking":1}`
  - Operate the analog out: `{"task": "/analogout_act", "analogoutid": 1, "analogoutval":1000}`
  - Operate the dac (e.g. lightsheet): `{"task": "/dac_act", "dac_channel": 19, "frequency":1, "offset":0, "amplitude":0, "clk_div": 10000}`


### Test the code using the Python interface

- Open a terminal in the folder where you downloaded this repository
- Navigate to the folder [PYTHON](https://github.com/openUC2/UC2-REST/tree/master/PYTHON)
- Install the following dependencies via `pip`:
```pip install requests python-opencv```
- Open the file [TEST_ESP32RestSerialAPI.py](https://github.com/openUC2/UC2-REST/blob/master/PYTHON/TEST_ESP32RestSerialAPI.py) 
- Adapt the `serialport`:
```
serialport = "/dev/cu.SLAB_USBtoUART"
serialport = "/dev/cu.SLAB_USBtoUART"
serialport = "/dev/cu.wchusbserial1430"
serialport = "COM3"
```
- Execute script in Python and check result
- In case of an error, file an Issue here 


## API defintion

This will come soon. 
In principle, every actuator/sensor should have three comonents:

- `*_act` => *action* -> do something
- `*_set` => *set* -> set parameters
- `*_get` => *get* -> get parameters

### Available hardware

- Stepper Motor (e.g. 2Wire)
- Analog Out (e.g. PWM)
- DAC (e.g. function generator for Galvos)
- Laser (e.g. TTL)
- State (e.g. information from the board) 
