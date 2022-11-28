import numpy as np

class LedMatrix(object):
    def __init__(self, parent, NLeds=64):
        #TOOD: This is for the LED matrix only!
        self.NLeds = NLeds
        #assuming we have a square grid!
        self.Nx = self.Ny = int(np.sqrt(NLeds))
        
        # we assume the pattern is binary (e.g. 0 or 1)
        self.ledpattern = np.zeros((self.Nx, self.Ny, 3))
        
        # this is a istance of the _parent32Client class
        self._parent = parent
        self.timeout = 1
        self.intensity = (255,255,255)
        
    
    '''
    ##############################################################################################################################
    LED ARRAY
    ##############################################################################################################################
    '''
    def send_LEDMatrix_array(self, led_pattern, timeout=1):
        '''
        Send an LED array pattern e.g. an RGB Matrix: led_pattern=np.zeros((3,8,8))
        '''
        path = '/ledarr_act'
        # Make sure LED strip is filled with matrix information
        if len(led_pattern.shape)<3:
            led_pattern = np.reshape(led_pattern, (led_pattern.shape[0], int(np.sqrt(led_pattern.shape[1])), int(np.sqrt(led_pattern.shape[1]))))
        led_pattern[:,1::2, :] = led_pattern[:,1::2, ::-1]
        payload = {
            "red": led_pattern[:,:,0].flatten().tolist(),
            "green": led_pattern[:,:,1].flatten().tolist(),
            "blue": led_pattern[:,:,2].flatten().tolist(),
            "arraySize": led_pattern.shape[1],
            "LEDArrMode": "array"
        }
        self._parent.logger.debug("Setting LED Pattern (array) ")
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def send_LEDMatrix_full(self, intensity = (255,255,255),timeout=1):
        '''
        set all LEDs with te same RGB value: intensity=(255,255,255)
        '''
        path = '/ledarr_act'
        payload = {
            "task":path,
            "red": int(intensity[0]),
            "green": int(intensity[1]),
            "blue": int(intensity[2]),
            "LEDArrMode": "full"
        }
        self._parent.logger.debug("Setting LED Pattern (full): "+ str(intensity))
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def send_LEDMatrix_special(self, pattern="left", intensity = (255,255,255),timeout=1):
        '''
        set all LEDs inside a certain pattern (e.g. left half) with the same RGB value: intensity=(255,255,255), rest 0
        '''
        path = '/ledarr_act'
        payload = {
            "red": int(intensity[0]),
            "green": int(intensity[1]),
            "blue": int(intensity[2]),
            "LEDArrMode": pattern
        }
        self._parent.logger.debug("Setting LED Pattern (full): "+ str(intensity))
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def send_LEDMatrix_single(self, indexled=0, intensity=(255,255,255), timeout=1):
        '''
        update only a single LED with a colour:  indexled=0, intensity=(255,255,255)
        '''
        path = '/ledarr_act'
        payload = {
            "red": int(intensity[0]),
            "green": int(intensity[1]),
            "blue": int(intensity[2]),
            "indexled": int(indexled),
            "LEDArrMode": "single"
        }
        self._parent.logger.debug("Setting LED PAttern: "+str(indexled)+" - "+str(intensity))
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def send_LEDMatrix_multi(self, indexled=(0), intensity=((255,255,255)), timeout=1):
        '''
        update a list of individual LEDs with a colour:  led_pattern=(1,2,6,11), intensity=((255,255,255),(125,122,1), ..)
        '''
        path = '/ledarr_act'
        payload = {
            "red": intensity[0],
            "green": intensity[1],
            "blue": intensity[2],
            "indexled": indexled,
            "LEDArrMode": "multi"
        }
        self._parent.logger.debug("Setting LED PAttern: "+str(indexled)+" - "+str(intensity))
        r = self._parent.post_json(path, payload, timeout=timeout)


    def get_LEDMatrix(self, timeout=1):
        '''
        get information about pinnumber and number of leds
        '''
        # TOOD: Not implemented yet
        path = "/ledarr_get"
        payload = {
            "task":path
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
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
        self.ledpattern = state*np.ones((self.Nx, self.Ny, 3))
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

    def setLEDArrayConfig(self, ledArrPin=4, ledArrNum=25):
        return self._parent.config.setLEDArrayConfig(ledArrPin=4, ledArrNum=25)
        