
# 
import uc2rest
import numpy as np
import time

port = "unknown"
port = "/dev/cu.SLAB_USBtoUART"
port = "/dev/cu.wchusbserial14310"
#port = "/dev/cu.wchusbserial1440"
port = "/dev/cu.wchusbserial110"

ESP32 = uc2rest.UC2Client(serialport=port)
# setting debug output of the ser'ial to true - all message will be printed
ESP32.serial.DEBUG=True

#%%
time.sleep(0)
#fully open
speed = 15000

ESP32.motor.move_xyzt(steps=(0,20000,-20000,0), speed=speed, is_blocking=True)
#time.sleep(.5)

# move to center
ESP32.motor.move_xyzt(steps=(0,-10000,10000,0), speed=speed, is_blocking=True)
#time.sleep(.5)

# open 
ESP32.motor.move_xyzt(steps=(0,5000,-5000,0), speed=speed, is_blocking=True)
#time.sleep(.5)

# close 
ESP32.motor.move_xyzt(steps=(0,-5000,5000,0), speed=speed, is_blocking=True)
#time.sleep(.5)

# both move left
ESP32.motor.move_xyzt(steps=(0,-5000,-5000,0), speed=speed, is_blocking=True)
#time.sleep(.5)

# both move right 
ESP32.motor.move_xyzt(steps=(0,5000,5000,0), speed=speed, is_blocking=True)
#time.sleep(.5)



# %
speed =1000
# both move left
ESP32.motor.move_xyzt(steps=(0,-5000,-5000,0), speed=speed, is_blocking=True)
#time.sleep(.5)

# both move right 
ESP32.motor.move_xyzt(steps=(0,5000,5000,0), speed=speed, is_blocking=True)
#time.sleep(.5)
# %%
