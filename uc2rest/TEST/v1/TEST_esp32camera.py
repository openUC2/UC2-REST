import uc2rest
import numpy as np
import time
import matplotlib.pyplot as plt

esp32cam = uc2rest.UC2Client(serialport="Unknown", identity="UC2_Camera")

# turn on led
esp32cam.camera.set_led(255)

# test Cam
for i in range(10):
    image = esp32cam.camera.get_frame()
    plt.figure(i)
    plt.imshow(image)
    plt.show()
    time.sleep(0.5)

# turn off led
esp32cam.camera.set_led(0)

print("done")