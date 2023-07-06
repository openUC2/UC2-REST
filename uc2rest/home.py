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
                  timeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease,
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)

    def home_y(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axis = 2 corresponds to 'Y'
        axis = 2
        self.home(axis=axis, 
                  timeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease, 
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)    
    
    def home_z(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axisa = 3 corresponds to 'Z'
        axis = 3
        self.home(axis=axis, 
                  timeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease, 
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)    
        
    def home(self, axis=None, timeout=None, speed=None, direction=None, endposrelease=None, endstoppolarity=None, isBlocking=False):
        '''
        axis = 0,1,2,3 or 'A, 'X','Y','Z'
        timeout => when to stop homing (it's a while loop on the MCU)
        speed => speed of homing (0...15000)
        direction => 1,-1 (left/right)
        endposrelease => how far to move after homing (0...3000)
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

        # construct json string
        path = "/home_act"

        payload = {
            "task": path,
            "home":{
                "steppers": [
                {
                 "stepperid": axis,
                 "timeout":timeout*1000,
                 "speed":speed,
                 "direction":direction,
                 "endposrelease":endposrelease, 
                 "endstoppolarity":endstoppolarity
                 }]
            }}
     
        # send json string
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        
        # wait until job has been done        
        time0=time.time()
        if isBlocking and self._parent.serial.is_connected:
            while True:
                time.sleep(0.05) # don'T overwhelm the CPU
                # see if already done
                try:
                    rMessage = self._parent.serial.serialdevice.readline().decode() # TODO: Make sure it's compatible with all motors running at the same time
                except Exception as e:
                    self._parent.logger.error(e)
                    rMessage = ""
                # check if message contains a motor that is done already
                if rMessage.find('isDone') >-1:
                    break
                if time.time()-time0>timeout:
                    break


        
        
        
        return r

