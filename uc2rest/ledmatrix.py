import numpy as np
import json
import time 
gTimeout = 0.5
class LedMatrix(object):
    def __init__(self, parent, NLeds=64):
        #TOOD: This is for the LED matrix only!
        self.NLeds = NLeds
        #assuming we have a square grid!
        self.Nx = self.Ny = int(np.sqrt(NLeds))
        
        # we assume the pattern is binary (e.g. 0 or 1)
        self.ledpattern = np.zeros((self.NLeds, 3))
        
        # this is a istance of the _parent32Client class
        self._parent = parent
        self.timeout = 1
        self.intensity = (255,255,255)
        
        self.ledArrayMode = {
            "array": 0,
            "full": 1,
            "single": 2,
            "off": 3,
            "left": 4,
            "right": 5,
            "top": 6,
            "bottom": 7,
            "multi": 8}
            
            
    
    '''
    ##############################################################################################################################
    LED ARRAY
    ##############################################################################################################################
    '''
    def send_LEDMatrix_array(self, led_pattern, timeout=gTimeout):
        '''
        Send an LED array pattern e.g. an RGB Matrix: led_pattern=np.zeros((3,8,8))
        '''
        path = '/ledarr_act'
        # if we have a 2d pattern => flatten
        if len(led_pattern.shape)==3:
            led_pattern=np.reshape(led_pattern, (np.prod(led_pattern.shape[0:2]),led_pattern.shape[2])) 

        # convert pattern strip to list of RGB values
        pattern_list = []
        for i in range(led_pattern.shape[0]):
            pattern_list.append({
                "id": i,
                "red": int(led_pattern[i,0]),
                "green": int(led_pattern[i,1]),
                "blue": int(led_pattern[i,2])                        
            })

        payload = {
            "task":path,
            "led": {
                "LEDArrMode": self.ledArrayMode["array"],
                "led_array": pattern_list
            }
        }
        self._parent.logger.debug("Setting LED Pattern (array) ")
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        
        return r

    def send_LEDMatrix_full(self, intensity = (255,255,255),timeout=gTimeout):
        '''
        set all LEDs with te same RGB value: intensity=(255,255,255)
        '''
        #{"task": "/ledarr_act","led": {"LEDArrMode": 1,"led_array": [{"blue": 255,"green": 255,"red": 255}]}}
        path = '/ledarr_act'
        payload = {
            "task":path,
            "led": {
                "led_array":[{
                    "red": int(intensity[0]),
                    "green": int(intensity[1]),
                    "blue": int(intensity[2])}],
                "LEDArrMode": self.ledArrayMode["full"]
            }
        }
        
        self._parent.logger.debug("Setting LED Pattern (full): "+ str(intensity))
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        return r

    def send_LEDMatrix_special(self, pattern="left", intensity = (255,255,255),timeout=gTimeout):
        '''
        set all LEDs inside a certain pattern (e.g. left half) with the same RGB value: intensity=(255,255,255), rest 0
        '''
        path = '/ledarr_act'
        payload = {
            "task":path,
            "led": {
            "red": int(intensity[0]),
            "green": int(intensity[1]),
            "blue": int(intensity[2]),
            "LEDArrMode": self.ledArrayMode[pattern]
            }
        }
        self._parent.logger.debug("Setting LED Pattern (full): "+ str(intensity))
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        return r

    def send_LEDMatrix_single(self, indexled=0, intensity=(255,255,255), timeout=gTimeout):
        '''
        update only a single LED with a colour:  indexled=0, intensity=(255,255,255)
        '''
        path = '/ledarr_act'
        payload = {
            "task":path,
            "led": {
                "LEDArrMode": self.ledArrayMode["single"],
                "led_array": [{
                    "id": indexled,
                    "red": int(intensity[0]),
                    "green": int(intensity[1]),
                    "blue": int(intensity[2])}]
            }
        }
        self._parent.logger.debug("Setting LED Pattern (single) ")
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        return r


    def get_LEDMatrix(self, timeout=gTimeout):
        '''
        get information about pinnumber and number of leds
        '''
        # TOOD: Not implemented yet
        path = "/ledarr_get"
        payload = {
            "task":path
        }
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        return r

    def setSingle(self, indexled, state):
        # Update the intensity of a single LED in a cartesian grid        
        ix = indexled//self.Nx
        iy = indexled%self.Nx
        self.ledpattern[ix, iy] = state # either [0, 1]
        # forward backward enumaration
        if ix%2 != 0:
           indexled = (ix*self.Nx)+(self.Ny-iy-1)
        self.send_LEDMatrix_single(indexled=indexled, intensity=np.array(state)*np.array(self.intensity), timeout=self.timeout)

        return self.ledpattern
    
    def setAll(self, state, intensity=None):
        # fast addressing
        # turns on all LEDs at a certain intensity
        if intensity is not None:
            self.intensity = intensity
        intensity2display = np.array(self.intensity)*np.array(state)
        self.send_LEDMatrix_full(intensity = intensity2display, timeout=self.timeout)
        self.ledpattern = state*np.ones((self.NLeds, 3))
        return self.ledpattern
    
    def setIntensity(self, intensity):
        self.intensity = intensity
        self.setPattern()
    
    def setPattern(self, ledpattern=None):
        # sends pattern with proper intensity
        if ledpattern is not None:
            self.ledpattern = ledpattern
        pattern2send = (self.ledpattern>=1)*self.intensity
        self.send_LEDMatrix_array(pattern2send, timeout=self.timeout)
        return self.ledpattern
    
    def getPattern(self):
        return self.ledpattern
        
    def set_led(self, colour=(0,0,0)):
        payload = {
            "red": colour[0],
            "green": colour[1],
            "blue": colour[2]
        }
        path = '/led'
        r = self._parent.post_json(path, payload)
        return r

    def set_ledpin(self, ledArrPin=4, ledArrNum=None):
        if ledArrNum is None:
            ledArrNum = self.NLeds
            
        path = "/ledarr_set"
        payload = {
            "task": path,
            "ledArrPin": ledArrPin,
            "ledArrNum": ledArrNum
        }
        r = self._parent.post_json(path, payload, getReturn=True)
        return r
        
    def get_ledpin(self):
        path = "/ledarr_get"
        payload = {
            "task": path,
        }
        r = self._parent.post_json(path, payload, getReturn=True, timeout=1)
        return r
            
    
    def setLEDArrayConfig(self, ledArrPin=4, ledArrNum=None):
        # make imswitch happy
        return self.set_ledpin(ledArrPin=ledArrPin, ledArrNum=ledArrNum)