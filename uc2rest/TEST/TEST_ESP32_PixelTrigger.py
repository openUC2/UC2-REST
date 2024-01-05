#%%
import uc2rest
import numpy as np
import time

port = "unknown"
port = "COM3"
port = "/dev/cu.SLAB_USBtoUART"
ESP32 = uc2rest.UC2Client(serialport=port, baudrate=500000, DEBUG=True)

''' ################
Start Stagescanning
################'''
ESP32.serial.DEBUG=True
ESP32.motor.startStageScanning(nStepsLine=100, dStepsLine=1, nTriggerLine=1, nStepsPixel=100, dStepsPixel=1, nTriggerPixel=1, delayTimeStep=10, nFrames=5)


#%%
''' ################
SetTrigger
################'''
# setting debug output of the serial to true - all message will be printed
ESP32.serial.DEBUG=True
ESP32.motor.setTrigger(axis="X", pin=1, offset=0, period=1)
ESP32.motor.setTrigger(axis="Y", pin=2, offset=0, period=1)

# now do snake scan
nLines = 100
speed = 1000
currentPosition = ESP32.motor.get_position()
nSteps = 1000
for i in range(nLines):
    print("Move")
    ESP32.motor.move_y(steps=2, speed=speed, is_blocking=True)
    
    if i%2==0:
        ESP32.motor.move_x(steps=nSteps, speed=speed, is_blocking=True)
    else:
        ESP32.motor.move_x(steps=-nSteps, speed=speed, is_blocking=True)
    
# move back to start position
ESP32.motor.move_xy(currentPosition[1:3], speed=1000, is_blocking=True)
ESP32.close()
