/* sample config file
  {
  "task":"/config_set", 
  "motXstp": 1,
  "motXdir": 2,
  "motYstp": 3,
  "motYdir": 4,
  "motZstp": 5,
  "motZdir": 6,
  "motAstp": 7,
  "motAdir": 8,
  "motEnable": 9,
  "ledArrPin": 0,
  "ledArrNum": 64,
  "digitalPin1":10,
  "digitalPin2":11,
  "analogPin1":12,
  "analogPin2":13,
  "analogPin3":14,
  "laserPin1":15,
  "laserPin2":16,
  "laserPin3":17,
  "dacFake1":18,
  "dacFake2":19,
  "identifier": "TEST",
  "ssid": "ssid",
  "PW": "PW"
  }
*/

const char* prefNamespace = "UC2";

const char* keyMotorXStepPin = "motXstp";
const char* keyMotorXDirPin = "motXdir";
const char* keyMotorYStepPin = "motYstp";
const char* keyMotorYDirPin = "motYdir";
const char* keyMotorZStepPin = "motZstp";
const char* keyMotorZDirPin = "motZdir";
const char* keyMotorAStepPin = "motAstp";
const char* keyMotorADirPin = "motAdir";
const char* keyMotorEnable = "motEnable";

const char* keyLEDArray = "ledArrPin";
const char* keyLEDNumLEDArray = "ledArrNum";

const char* keyDigital1Pin = "digitalPin1";
const char* keyDigital2Pin = "digitalPin2";

const char* keyAnalog1Pin = "analogPin1";
const char* keyAnalog2Pin = "analogPin2";
const char* keyAnalog3Pin = "analogPin3";

const char* keyLaser1Pin = "laserPin1";
const char* keyLaser2Pin = "laserPin2";
const char* keyLaser3Pin = "laserPin3";

const char* keyDACfake1Pin = "dacFake1";
const char* keyDACfake2Pin = "dacFake2";

const char* keyIdentifier = "identifier";

const char* keyWifiSSID = "ssid";
const char* keyWifiPW = "PW";
