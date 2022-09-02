from ESP32Client import ESP32Client, ledmatrix
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

#%%
ESP32.send_LEDMatrix_full(intensity = (0,0,0),timeout=1)

# High level matrix addressing
mLEDmatrix = ledmatrix(ESP32, NLeds=16)
mLEDmatrix.single(0, (255,255,255))
mLEDmatrix.single(4, (255,255,255))
mLEDmatrix.single(8, (255,255,255))
mLEDmatrix.single(12, (255,255,255))

intensity = (125,125,125)
led_pattern = np.diag(np.ones(4))
led_pattern = np.expand_dims(led_pattern,-1)*intensity
led_pattern = np.transpose(led_pattern,(2,0,1))
mLEDmatrix.pattern(led_pattern)

mLEDmatrix.all((255,255,255))


#%%
# turn on random LEDs   
Nx=4
Ny=4
led_pattern = np.abs(np.int8(np.random.randn(3,Nx*Ny)*125))
ESP32.send_LEDMatrix_array(led_pattern, timeout=1)

# turn on ring of LEDs
intensity = np.array((25,25,25))
led_pattern = np.array(((0,1,1,0),(1,1,1,1),(1,1,1,1),(0,1,1,0)))
led_pattern = np.expand_dims(led_pattern,-1)*intensity
led_pattern = np.transpose(led_pattern,(2,0,1))
ESP32.send_LEDMatrix_array(led_pattern, timeout=1)

# turn on diagonal 
intensity = (125,125,125)
led_pattern = np.diag(np.ones(Nx))

led_pattern = np.expand_dims(led_pattern,-1)*intensity
led_pattern = np.transpose(led_pattern,(2,0,1))

ESP32.send_LEDMatrix_array(led_pattern, timeout=1)

#%% iterate over all LEDs
for i in range(0,Nx):
    for j in range(0,Ny):
        led_pattern = np.zeros((3,Nx,Ny))
        led_pattern[:,i,j] = intensity
        ESP32.send_LEDMatrix_array(led_pattern, timeout=1)

# tur off all LEDs
ESP32.send_LEDMatrix_full(intensity = (0,0,0))

# turn on single LED 
ESP32.send_LEDMatrix_single(indexled=10, intensity=(255,255,255), timeout=1)

# turn on a selected list of LEDs
ESP32.send_LEDMatrix_multi(indexled=(0,2,6), intensity=((255,255,255),(255,255,255),(255,255,255)), timeout=1)

#%%
