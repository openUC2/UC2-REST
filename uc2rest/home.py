import time
import json        
        
class Home(object):
    ## Laser
    def __init__(self, parent):
        self._parent = parent
        self.endposrelease = 3000
        self.direction = 1
        self.speed = 15000
        self.timeout = 20000
        self.endstoppolarity = 1
        
    def home_x(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axis = 1 corresponds to 'X'
        axis = 1
        self.home(axis=axis, 
                  endstoptimeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease,
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)

    def home_y(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axis = 2 corresponds to 'Y'
        axis = 2
        self.home(axis=axis, 
                  endstoptimeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease, 
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)    
    
    def home_z(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axisa = 3 corresponds to 'Z'
        axis = 3
        self.home(axis=axis, 
                  endstoptimeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease, 
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)    
        
    def home_a(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axis = 0 corresponds to 'A'
        axis = 0
        self.home(axis=axis,
                  endstoptimeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease, 
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)    
    
    def home(self, axis=None, timeout=None, speed=None, direction=None, endposrelease=None, endstoppolarity=None, endstoptimeout=10000, isBlocking=False, preMove=True):
        '''
        axis = 0,1,2,3 or 'A, 'X','Y','Z'
        timeout => when to stop homing (it's a while loop on the MCU)
        speed => speed of homing (0...15000)
        direction => 1,-1 (left/right)
        endposrelease => how far to move after homing (0...3000)
        preMove => the motor will first move by some steps in the opposite direction before homing, this is useful to avoid false triggering of the endstop
        '''
        
        # default values
        if speed is None:
            speed = self.speed
        if direction is None:
            direction = self.direction
        if endposrelease is None:
            endposrelease = self.endposrelease
        if timeout is None:
            timeout = self.timeout
        if endstoppolarity is None:
            endstoppolarity = self.endstoppolarity

        if direction not in [-1,1]:
            direction = 1

        if preMove:
            # first move in the opposite direction
            if direction == 1:
                preMoveDirection = -1
            else:
                preMoveDirection = 1
            
            # move away from endstop
            if axis == 1 or axis == "X":
                self._parent.motor.move_x(steps=preMoveDirection*100, speed=self.speed, is_blocking=True, is_absolute=False, is_enabled=True)
            elif axis == 2 or axis == "Y":
                self._parent.motor.move_y(steps=preMoveDirection*100, speed=self.speed, is_blocking=True, is_absolute=False, is_enabled=True)
            elif axis == 3 or axis == "Z":
                self._parent.motor.move_z(steps=preMoveDirection*100, speed=self.speed, is_blocking=True, is_absolute=False, is_enabled=True)
            elif axis == 0 or axis == "A":
                self._parent.motor.move_a(steps=preMoveDirection*100, speed=self.speed, is_blocking=True, is_absolute=False, is_enabled=True)
            else:   
                raise ValueError("Invalid axis. Use 'X', 'Y', 'Z', or 'A'.")
            time.sleep(0.5)
        # construct json string
        path = "/home_act"

        payload = {
            "task": path,
            "home":{
                "steppers": [
                {
                 "stepperid": axis,
                 "timeout":endstoptimeout,
                 "speed":abs(speed),
                 "direction":direction,
                 "endstoppolarity":endstoppolarity
                 }]
            }}
     
        timeout = timeout if isBlocking else 0
        nResponses = 2 # one for command received, one for home reached
        
        # if we get a return, we will receive the latest position feedback from the driver  by means of the axis that moves the longest
        r = self._parent.post_json(path, payload, getReturn=isBlocking, timeout=timeout, nResponses=nResponses)

        return r

