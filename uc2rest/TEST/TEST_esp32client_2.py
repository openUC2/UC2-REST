import uc2rest
import numpy as np
import time

esp32 = uc2rest.UC2Client(serialport="Unknown")

# test LED
esp32.led.setLEDArrayConfig(ledArrPin=4, ledArrNum=25)
led_pattern = np.zeros((1, 5, 5, 3), dtype=np.uint8)
esp32.led.send_LEDMatrix_array(led_pattern=led_pattern, timeout=1)
esp32.led.send_LEDMatrix_full(intensity=(255, 0, 0), timeout=1)
esp32.led.send_LEDMatrix_single(indexled=0, intensity=(0, 255, 0), timeout=1)
esp32.led.get

# test Motor
esp32.motor.move_x(steps=1000, speed=1000)
esp32.motor.move_y(steps=1000, speed=1000)
esp32.motor.move_z(steps=1000, speed=1000)
esp32.motor.move_t(steps=1000, speed=1000)
esp32.motor.move_xyzt(steps=1000, speed=1000)
esp32.motor.move_forever(speed=(100,0,0), is_stop=False)
esp32.motor.move_forever(speed=(100,0,0), is_stop=True)

esp32.motor.set_motor_maxSpeed(axis=0, maxSpeed=10000)
esp32.motor.set_motor_currentPosition(axis=0, currentPosition=10000)
esp32.motor.set_motor_acceleration(axis=0, acceleration=10000)
esp32.motor.set_motor_enable(is_enable=1)
esp32.motor.set_direction(axis=1, sign=1, timeout=1)
position = esp32.motor.get_position(axis=1, timeout=1)
esp32.motor.set_position(axis=1, position=0, timeout=1)

# test laser 
esp32.laser.set_laser(channel=1, value=1000, despeckleAmplitude=0.5, despecklePeriod=10, timeout=20, is_blocking = True)
esp32.laser.set_laser(channel=1, value=1000, despeckleAmplitude=0.5, despecklePeriod=10, timeout=20, is_blocking = True)
esp32.laser.set_laser(channel=1, value=1000, despeckleAmplitude=0.5, despecklePeriod=10, timeout=20, is_blocking = True)


# test state
_state = esp32.state.get_state()
print(_state)
esp32.state.set_state(debug=False)
_mode = esp32.state.isControllerMode()
print(_mode)
esp32.state.espRestart()
time.sleep(5)
esp32.state.setControllerMode(isController=True)
_busy = esp32.state.isBusy()
print(_busy)


# wifi
esp32.wifi.scanWifi()


