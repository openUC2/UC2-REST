from astropy.units import Quantity
import astropy.units as u
import uc2rest
import numpy as np
import time


class UC2XYStage:
    def __init__(self, step_size_x: Quantity[u.um], step_size_y: Quantity[u.um], serialport: str = None, baudrate: int = 115200, DEBUG: bool = False):
        super().__init__()
        
        # initialize the UC2-Serial interface # TODO for a shared device (e.g. illumination/laser/motor) 
        # we would need to have a "microscope" class instead
        self.ESP32 = uc2rest.UC2Client(serialport=serialport, 
                                       baudrate=baudrate, 
                                       DEBUG=DEBUG)
        
        self._step_size_x = step_size_x.to(u.um)
        self._step_size_y = step_size_y.to(u.um)
        
        # 
        self._y = 0.0 * u.um
        self._x = 0.0 * u.um
        self.isBusy = False
        
        
        # homing configurations 
        self.home_x_speed = 15000
        self.home_x_direction = 1
        self.home_x_endposrelease = 3000
        self.home_x_timeout = 20
        self.home_y_speed = 15000
        self.home_y_direction = 1
        self.home_y_endposrelease = 3000
        self.home_y_timeout = 20
        
        # motor configurations
        self.step_top_micrometer_x = 1/3200
        self.step_top_micrometer_y = 1/3200
        self.speed_x = 15000
        self.speed_y = 15000
        

    def home(self):
        self.isBusy = True
        self.ESP32.home.home_x(speed=self.home_x_speed, direction=self.home_x_direction, endposrelease=self.home_x_endposrelease, timeout=self.home_x_timeout, isBlocking=True)
        time.sleep(0.1)
        self.ESP32.home.home_y(speed=self.home_y_speed, direction=self.home_y_direction, endposrelease=self.home_y_endposrelease, timeout=self.home_y_timeout, isBlocking=True)
        self._x = 0.0 * u.um
        self._y = 0.0 * u.um
        self.isBusy = False

    def busy(self):
        return self.isBusy # TODO: Need to be inside a lock?

    @property
    def x(self) -> Quantity[u.um]:
        return self._x

    @x.setter
    def x(self, value: Quantity[u.um]):
        # assuming this is a relative movement
        self.isBusy = True
        former = self._x
        self.ESP32.motor.move_x(steps=value.to(u.um)/self.step_top_micrometer_x, speed=self.speed_x, is_blocking=True, is_absolute=False)
        self._x = value.to(u.um) + former
        self.isBusy = False
        
    @property
    def y(self) -> Quantity[u.um]:
        return self._y

    @y.setter
    def y(self, value: Quantity[u.um]):
        # assuming this is a relative movement
        self.isBusy = True
        former = self._y
        self.ESP32.motor.move_y(steps=value.to(u.um)/self.step_top_micrometer_x, speed=self.speed_y, is_blocking=True, is_absolute=False)
        self._y = value.to(u.um) + former
        self.isBusy = False
        
    @property
    def step_size_x(self) -> Quantity[u.um]:
        return self._step_size_x

    @step_size_x.setter
    def step_size_x(self, value: Quantity[u.um]):
        self._step_size_x = value.to(u.um)

    @property
    def step_size_y(self) -> Quantity[u.um]:
        return self._step_size_y

    @step_size_y.setter
    def step_size_y(self, value: Quantity[u.um]):
        self._step_size_y = value.to(u.um)


devices = {'stage': UC2XYStage(1 * u.um, 1 * u.um)}

if __name__ == "__main__":
    import sys
    import os

    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    from bootstrap import PyDevice

    device = PyDevice(devices['stage'])
    print(device)
    assert device.device_type == 'XYStage'