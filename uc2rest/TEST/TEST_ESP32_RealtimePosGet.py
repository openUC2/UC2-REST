#%%
import uc2rest
import numpy as np
import time

port = "unknown"
port = "/dev/cu.SLAB_USBtoUART"
ESP32 = uc2rest.UC2Client(serialport=port, DEBUG=False)
ESP32.motor.move_x(steps=100000, speed=10000, is_blocking=False)
print("Start")
t0 = time.time()
for i in range(10):
    position1 = ESP32.motor.get_position(timeout=1)
    print(position1)
    print(t0-time.time())
    t0 = time.time()
    
ESP32.close()