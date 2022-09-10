import uc2rest as uc2

ESP32 = uc2.ESP32Client(serialport="unknown")


mConfig = ESP32.loadConfig()

mConfig = {
    "motXstp": 0,
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
    "PW": "PW"}

ESP32.config.setConfig(mConfig)

mConfig = ESP32.loadConfig()

