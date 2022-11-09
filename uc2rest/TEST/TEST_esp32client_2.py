import uc2rest
import numpy as np
import time

port = "unknown"
port = "/dev/cu.SLAB_USBtoUART"
port = "/dev/cu.wchusbserial14310"
#port = "/dev/cu.wchusbserial1440"

esp32 = uc2rest.UC2Client(serialport=port)

esp32.serial.DEBUG=True

# test Serial
test_cmd = "{'task': '/motor_get'}"
esp32.serial.writeSerial(test_cmd)
cmd_return = esp32.serial.readSerial()
print(cmd_return)

# setup motors (according to WEMOS R32 D1)
esp32.motor.set_motor(stepperid = 1, position = 0, stepPin = 26, dirPin=16, enablePin=12, maxPos=None, minPos=None, acceleration=None, isEnable=1)
esp32.motor.set_motor(stepperid = 2, position = 0, stepPin = 25, dirPin=27, enablePin=12, maxPos=None, minPos=None, acceleration=None, isEnable=1)
esp32.motor.set_motor(stepperid = 3, position = 0, stepPin = 17, dirPin=14, enablePin=12, maxPos=None, minPos=None, acceleration=None, isEnable=1)
esp32.motor.set_motor(stepperid = 0, position = 0, stepPin = 19, dirPin=18, enablePin=12, maxPos=None, minPos=None, acceleration=None, isEnable=1)

esp32.motor.set_motor_currentPosition(axis=0, currentPosition=10000)
esp32.motor.set_motor_acceleration(axis=0, acceleration=10000)
esp32.motor.set_motor_enable(is_enable=1)
esp32.motor.set_direction(axis=1, sign=1, timeout=1)
esp32.motor.set_position(axis=1, position=0, timeout=1)

# wait to settle
time.sleep(2)

# test Motor
position1 = esp32.motor.get_position(timeout=1)
print(position1)
esp32.motor.move_x(steps=10000, speed=10000, is_blocking=True)
esp32.motor.move_y(steps=1000, speed=1000, is_blocking=True)
esp32.motor.move_z(steps=1000, speed=1000, is_blocking=True)
esp32.motor.move_t(steps=1000, speed=1000)
esp32.motor.move_xyzt(steps=(0,10000,10000,0), speed=10000, is_blocking=True)
esp32.motor.move_xyzt(steps=(0,0,0,0), speed=10000, is_absolute=True, is_blocking=True)
esp32.motor.move_forever(speed=(0,100,0,0), is_stop=False)
time.sleep(1)
esp32.motor.move_forever(speed=(0,0,0,0), is_stop=True)

position2 = esp32.motor.get_position(timeout=1)
print(position2)

# test laser 
esp32.laser.set_laser(channel=1, value=1000, despeckleAmplitude=0, despecklePeriod=10, timeout=20, is_blocking = True)
esp32.laser.set_laser(channel=2, value=1000, despeckleAmplitude=0, despecklePeriod=10, timeout=20, is_blocking = True)
esp32.laser.set_laser(channel=3, value=1000, despeckleAmplitude=0, despecklePeriod=10, timeout=20, is_blocking = True)


# test LED
esp32.led.setLEDArrayConfig(ledArrPin=4, ledArrNum=25)
led_pattern = np.zeros((1, 5, 5, 3), dtype=np.uint8)
esp32.led.send_LEDMatrix_array(led_pattern=led_pattern, timeout=1)
esp32.led.send_LEDMatrix_full(intensity=(255, 0, 0), timeout=1)
esp32.led.send_LEDMatrix_single(indexled=0, intensity=(0, 255, 0), timeout=1)


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


