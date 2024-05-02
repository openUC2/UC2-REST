import numpy as np
import json
import time 
gTimeout = 2
class LedMatrix(object):
    def __init__(self, parent, NLeds=64):
        #TOOD: This is for the LED matrix only!
        self.NLeds = NLeds
        #assuming we have a square grid!
        self.Nx = self.Ny = int(np.sqrt(NLeds))
        
        # we assume the pattern is binary (e.g. 0 or 1)
        self.ledpattern = np.ones((self.NLeds, 3))*-1 # not set yet
        
        # this is a istance of the _parent32Client class
        self._parent = parent
        self.timeout = 1
        self.intensity = (255,255,255)
        
        self.ledArrayModes = {
            "array": 0,
            "full": 1,
            "single": 2,
            "off": 3,
            "left": 4,
            "right": 5,
            "top": 6,
            "bottom": 7,
            "multi": 8}

        self.currentLedArrayMode = "full"            
            
    
    '''
    ##############################################################################################################################
    LED ARRAY
    ##############################################################################################################################
    '''
    def send_LEDMatrix_array(self, led_pattern, getReturn = True, timeout=gTimeout):
        '''
        Send an LED array pattern e.g. an RGB Matrix: led_pattern=np.zeros((3,8,8))
        '''
        path = '/ledarr_act'

        # convert pattern strip to list of RGB values
        if not type(led_pattern) is list:
            # if we have a 2d pattern => flatten
            if len(led_pattern.shape)==3:
                led_pattern=np.reshape(led_pattern, (np.prod(led_pattern.shape[0:2]),led_pattern.shape[2])) 

            pattern_list = []
            for i in range(led_pattern.shape[0]):
                pattern_list.append({
                    "id": i,
                    "r": int(led_pattern[i,0]),
                    "g": int(led_pattern[i,1]),
                    "b": int(led_pattern[i,2])                        
                })
        else:
            pattern_list = led_pattern

        payload = {
            "task":path,
            "led": {
                "LEDArrMode": self.ledArrayModes["array"],
                "led_array": pattern_list
            }
        }
        #self._parent.logger.debug("Setting LED Pattern (array) ")
        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        if not getReturn or timeout==0:
            r = {"success": 1}
        self.currentLedArrayMode = "array"            
        return r

    def send_LEDMatrix_full(self, intensity = (255,255,255), getReturn=True, timeout=gTimeout):
        '''
        set all LEDs with te same RGB value: intensity=(255,255,255)
        '''
        #{"task": "/ledarr_act","led": {"LEDArrMode": 1,"led_array": [{"b": 255,"g": 255,"r": 255}]}}
        path = '/ledarr_act'
        payload = {
            "task":path,
            "led": {
                "led_array":[{
                    "id": 0,
                    "r": int(intensity[0]),
                    "g": int(intensity[1]),
                    "b": int(intensity[2])}],
                "LEDArrMode": self.ledArrayModes["full"]
            }
        }
        #{"task":"/ledarr_act", "led":{"LEDArrMode":1, "led_array":[{"id":0, "r":255, "g":255, "b":255}]}}
        #{"task":"/ledarr_act", "led":{"LEDArrMode":1, "led_array":[{"id":0, "r": 255, "g": 255, "b": 255}]}}
        #self._parent.logger.debug("Setting LED Pattern (full): "+ str(intensity))
        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        if not getReturn or timeout==0:
            r = {"success": 1}
        self.currentLedArrayMode = "full"            
        return r

    def send_LEDMatrix_special(self, pattern="left", intensity = (255,255,255), getReturn = True, timeout=gTimeout):
        '''
        set all LEDs inside a certain pattern (e.g. left half) with the same RGB value: intensity=(255,255,255), rest 0
        '''
        path = '/ledarr_act'
        payload = {
            "task":path,
            "led": {
            "r": int(intensity[0]),
            "g": int(intensity[1]),
            "b": int(intensity[2]),
            "LEDArrMode": self.ledArrayModes[pattern]
            }
        }
        self._parent.logger.debug("Setting LED Pattern (full): "+ str(intensity))
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        if not getReturn or timeout==0:
            r = {"success": 1}
        self.currentLedArrayMode = "special"            
        return r

    def send_LEDMatrix_single(self, indexled=0, intensity=(255,255,255), getReturn = True, timeout=gTimeout):
        '''
        update only a single LED with a colour:  indexled=0, intensity=(255,255,255)
        '''
        path = '/ledarr_act'
        payload = {
            "task":path,
            "led": {
                "LEDArrMode": self.ledArrayModes["single"],
                "led_array": [{
                    "id": indexled,
                    "r": int(intensity[0]),
                    "g": int(intensity[1]),
                    "b": int(intensity[2])}]
            }
        }
        self._parent.logger.debug("Setting LED Pattern (single) ")
        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        if not getReturn or timeout==0:
            r = {"success": r}
        self.currentLedArrayMode = "single"            
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
        self.ledpattern[indexled] = (state,state,state) # either [0, 1]
        # forward backward enumaration
        if ix%2 != 0:
           indexled = (ix*self.Nx)+(self.Ny-iy-1)
        self.send_LEDMatrix_single(indexled=indexled, intensity=np.array(state)*np.array(self.intensity), timeout=self.timeout)

        return self.ledpattern
    
    def setAll(self, state, intensity=None, getReturn=True):
        # fast addressing
        # turns on all LEDs at a certain intensity
        state = np.sum(state)>0
        if intensity is not None:
            self.intensity = intensity
        intensity2display = np.array(self.intensity)*np.array(state)
        self.send_LEDMatrix_full(intensity = intensity2display, getReturn=getReturn)
        self.ledpattern = state*np.ones((self.NLeds, 3))
        return self.ledpattern
    
    def setIntensity(self, intensity, getReturn=True):
        self.intensity = intensity
        self.setPattern(getReturn=getReturn)
    
    def setPattern(self, getReturn=True, ledpattern=None):
        # sends pattern with proper intensity
        if ledpattern is not None:
            self.ledpattern = ledpattern
        pattern2send = (self.ledpattern>=1)*self.intensity
        if np.sum(self.ledpattern, 0)[0]==self.ledpattern.shape[0]:
            # turn on all - faster! 
            self.send_LEDMatrix_full(pattern2send[0,:], getReturn=getReturn)
        else:
            # set individual pattern - slower
            self.send_LEDMatrix_array(pattern2send, getReturn=getReturn)
        return self.ledpattern
    
    def getPattern(self):
        return self.ledpattern
        
    def set_led(self, colour=(0,0,0)):
        payload = {
            "r": colour[0],
            "g": colour[1],
            "b": colour[2]
        }
        path = '/led'
        r = self._parent.post_json(path, payload)
        return r
        
    def get_ledpin(self):
        path = "/ledarr_get"
        r = self._parent.get_json(path, getReturn=True, timeout=1)
        return r
