import uc2rest
import numpy as np
import time

port = "unknown"
port = "/dev/cu.SLAB_USBtoUART"
port = "/dev/cu.wchusbserial14310"
#port = "/dev/cu.wchusbserial1440"
port = "/dev/cu.wchusbserial110"

ESP32 = uc2rest.UC2Client(serialport=port)
# setting debug output of the serial to true - all message will be printed
ESP32.serial.DEBUG=True

i=0
while(True):
    i+=1
    print("I'm running iteration: ", i)
    ESP32.motor.set_motor_enable(axis=0, is_enable=0)
    # test LED
    ESP32.led.send_LEDMatrix_full(intensity=(255, 255, 255))
    time.sleep(0.5)
    ESP32.led.send_LEDMatrix_full(intensity=(0, 0, 0))
    # set laser values
    ESP32.laser.set_laser(channel=1, value=1000, despeckleAmplitude=0, despecklePeriod=10, timeout=20, is_blocking = True)
    ESP32.laser.set_laser(channel=2, value=1000, despeckleAmplitude=0, despecklePeriod=10, timeout=20, is_blocking = True)
    ESP32.laser.set_laser(channel=3, value=1000, despeckleAmplitude=0, despecklePeriod=10, timeout=20, is_blocking = True)



