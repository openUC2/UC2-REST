#%%
import uc2rest
import numpy as np
import time

port = "unknown"
port = "/dev/cu.SLAB_USBtoUART"
ESP32 = uc2rest.UC2Client(serialport=port, baudrate=500000, DEBUG=True)
time.sleep(2)
#ESP32.serial.sendMessage('{"task":"/home_act", "home": {"steppers": [{"stepperid":1, "timeout": 20000, "speed": 15000, "direction":1, "endposrelease":3000}]}}')
# display random pattern
for i in range(10):
    led_pattern = np.random.randint(0,55, (64,3))
    ESP32.led.send_LEDMatrix_array(led_pattern=led_pattern, getReturn=True, timeout=4)
    
# 
''' ################
LED 
################'''
# test LED
ESP32.led.send_LEDMatrix_full(intensity=(255, 255, 255))
ESP32.led.send_LEDMatrix_full(intensity=(0, 0, 0))

# single LED
for iLED in range(5):
    # timeout = 0 means no timeout => mResult will be rubish!
    mResult = ESP32.led.send_LEDMatrix_single(indexled=iLED, intensity=(255, 255, 255))#, timeout=0.)
    mResult = ESP32.led.send_LEDMatrix_single(indexled=iLED, intensity=(0, 0, 0))#, timeout=0.)

# display random pattern
for i in range(50):
    led_pattern = np.random.randint(0,55, (48,3))
    ESP32.led.send_LEDMatrix_array(led_pattern=led_pattern, getReturn=False)

ESP32.close()

