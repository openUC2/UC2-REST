#%%
import uc2rest
import numpy as np
import time

port = "unknown"
#port = "/dev/cu.SLAB_USBtoUART"
#port = "COM3"
port = "/dev/cu.SLAB_USBtoUART"
baudrate = 912600
print("start")
ESP32 = uc2rest.UC2Client(serialport=port, baudrate=baudrate, DEBUG=False, skipFirmwareCheck=False)
#ESP32.serial.sendMessage('{"task":"/home_act", "home": {"steppers": [{"stepperid":1, "timeout": 20000, "speed": 15000, "direction":1, "endposrelease":3000}]}}')

for i in range(15):
        
    temp_dict = ESP32.i2c.read_sht45()
    light_dict = ESP32.i2c.read_tsl2591()


    print("SHT45: ", temp_dict)
    print("TSL2591: ", light_dict)