#%%
import uc2rest
import numpy as np
import time

port = "/dev/cu.SLAB_USBtoUART"
port = "unknown"
ESP32 = uc2rest.UC2Client(serialport=port, baudrate=115200, DEBUG=True)

# open gripper
for i in range(10):
    ESP32.gripper.open(isBlocking=True)
    # close gripper^
    time.sleep(0.5)
    ESP32.gripper.close(isBlocking=True)
    time.sleep(0.5)
# set gripper angle
ESP32.gripper.setAngle(90, isBlocking=True)


ESP32.close()