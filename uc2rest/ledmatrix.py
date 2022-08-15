import numpy as np

class ledmatrix(object):
    def __init__(self, ESP, NLeds=64):
        #TOOD: This is for the LED matrix only!
        self.NLeds = NLeds
        #assuming we have a square grid!
        self.Nx = self.Ny = int(np.sqrt(NLeds))
        
        # we assume the pattern is binary (e.g. 0 or 1)
        self.ledpattern = np.zeros((self.Nx, self.Ny, 3))
        
        # this is a istance of the ESP32Client class
        self.ESP = ESP
        self.timeout = 1
        self.intensity = (255,255,255)
        
    def setSingle(self, indexled, state):
        # Update the intensity of a single LED in a cartesian grid        
        ix = indexled//self.Nx
        iy = indexled%self.Nx
        self.ledpattern[ix, iy] = state # either [0, 1]
        # forward backward enumaration
        if ix%2 != 0:
           indexled = (ix*self.Nx)+(self.Ny-iy-1)
        self.ESP.send_LEDMatrix_single(indexled=indexled, intensity=np.array(state)*np.array(self.intensity), timeout=self.timeout)

        return self.ledpattern
    
    def pattern(self, ledpattern):
        self.ESP.send_LEDMatrix_array(ledpattern, timeout=self.timeout)
        self.ledpattern = ledpattern
        return self.ledpattern
    
    def setAll(self, state):
        # fast addressing
        intensity2display = np.array(self.intensity)*np.array(state)
        self.ESP.send_LEDMatrix_full(intensity = intensity2display, timeout=self.timeout)
        self.ledpattern = intensity2display*np.ones((self.Nx, self.Ny, 3))
        return self.ledpattern
    
    def setIntensity(self, intensity):
        self.intensity = intensity
        self.setPattern()
    
    def setPattern(self):
        # sends pattern with proper intensity
        pattern2send = self.ledpattern*self.intensity
        self.ESP.send_LEDMatrix_array(pattern2send, timeout=self.timeout)
        