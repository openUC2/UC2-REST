from ESP32Client import ESP32Client  
ESP32 = ESP32Client(serialport="unknown")

# move and measure
print(ESP32.get_position(axis=1))
ESP32.move_x(steps=1000, speed=1000, is_blocking=True, is_absolute=False, is_enabled=False)
print(ESP32.get_position(axis=1))
ESP32.move_x(steps=0, speed=1000, is_blocking=True, is_absolute=True, is_enabled=False)
print(ESP32.get_position(axis=1))


# set all LEDs to a certain RGB value
ESP32.send_LEDMatrix_full(intensity = (0,255,255),timeout=1)


# set a funny pattern
import numpy as np

Nx=8
Ny=8
led_pattern = np.abs(np.int8(np.random.randn(3,Nx*Ny)*255))
ESP32.send_LEDMatrix_array(led_pattern, timeout=1)

