import uc2rest
import numpy as np
import time

esp32 = uc2rest.UC2Client(serialport="Unknown")

# test LED
if(0):
    esp32.led.setLEDArrayConfig(ledArrPin=4, ledArrNum=25)
esp32.led.send_LEDMatrix_single(indexled=0, intensity=(0, 255, 0), timeout=1)


# test Motor
esp32.motor.move_stepper(steps=(0,0,0,1), speed=(0,0,0,200)) 
esp32.motor.move_forever(speed=(0,0,0,200), is_stop=False)
time.sleep(1)

# test laser 
esp32.laser.set_laser(channel=2, value=1000)
time.sleep(1)

# test dac
esp32.galvo.set_dac(channel=1, frequency=1, offset=0, amplitude=1/2, clk_div=0)
time.sleep(1)
esp32.motor.move_forever(speed=(0,0,0,2000), is_stop=False)