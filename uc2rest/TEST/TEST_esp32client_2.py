import uc2rest
import numpy as np

esp32 = uc2rest.ESP32Client(serialport="Unknown")

# test LED
led_pattern = np.zeros((1, 5, 5, 3), dtype=np.uint8)
esp32.led.send_LEDMatrix_array(led_pattern=led_pattern, timeout=1)
esp32.led.send_LEDMatrix_full(intensity=(255, 0, 0), timeout=1)
esp32.led.send_LEDMatrix_single(indexled=0, intensity=(0, 255, 0), timeout=1)

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