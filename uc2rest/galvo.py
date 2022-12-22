class Galvo(object):
    def __init__(self, parent=None):
        '''
        defaults:
            dac->Setup(DAC_CHANNEL_1, 0, 1, 0, 0, 2);
            dac->Setup(dac_channel, clk_div, frequency, scale, phase, invert);
      '''
        self.channel= 1
        self.frequency = 1
        self.offset = 0
        self.amplitude = amplitude=1/2
        self.clk_div = 0
        self.path = "/dac_act"
        
        self._parent = parent

    
    def set_dac(self, channel=1, frequency=1, offset=0, amplitude=1/2, clk_div=0, timeout=1):
        
        path = "/dac_act"
        payload = {
            "task": path,
            "dac_channel": channel, # 1 or 2
            "frequency": frequency,
            "offset": offset,
            "amplitude":amplitude,
            "clk_div": clk_div
        }
                        
        self._parent.post_json(path, payload, timeout=timeout, getReturn=False)

    '''
    ##############################################################################################################################
    SCANNER
    ##############################################################################################################################
    '''
    def set_scanner_pattern(self, numpyPattern, scannernFrames=1,
            scannerLaserVal=32000,
            scannerExposure=500, scannerDelay=500, is_blocking = False):

        scannerMode="pattern"
        path = '/scanner_act'
        arraySize = int(np.prod(numpyPattern.shape))
        payload = {
            "task":path,
            "scannernFrames":scannernFrames,
            "scannerMode":scannerMode,
            "arraySize":arraySize,
            "i":numpyPattern.flatten().tolist(),
            "scannerLaserVal":scannerLaserVal,
            "scannerExposure":scannerExposure,
            "scannerDelay":scannerDelay,
            "isblock": is_blocking
            }

        r = self.post_json(path, payload)
        return r

    def set_scanner_classic(self, scannernFrames=100,
            scannerXFrameMin=0, scannerXFrameMax=255,
            scannerYFrameMin=0, scannerYFrameMax=255,
            scannerEnable=0, scannerxMin=1,
            scannerxMax=5, scanneryMin=1,
            scanneryMax=5, scannerXStep=25,
            scannerYStep=25, scannerLaserVal=32000,
            scannerExposure=500, scannerDelay=500):

        scannerModec="classic",
        path = '/scanner_act'
        payload = {
            "task":path,
            "scannernFrames":scannernFrames,
            "scannerMode":scannerModec,
            "scannerXFrameMin":scannerXFrameMin,
            "scannerXFrameMax":scannerXFrameMax,
            "scannerYFrameMin":scannerYFrameMin,
            "scannerYFrameMax":scannerYFrameMax,
            "scannerEnable":scannerEnable,
            "scannerxMin":scannerxMin,
            "scannerxMax":scannerxMax,
            "scanneryMin":scanneryMin,
            "scanneryMax":scanneryMax,
            "scannerXStep":scannerXStep,
            "scannerYStep":scannerYStep,
            "scannerLaserVal":scannerLaserVal,
            "scannerExposure":scannerExposure,
            "scannerDelay":scannerDelay}

        r = self.post_json(path, payload)
        return r


    def set_galvo_freq(self, axis=1, value=1000):
        if axis+1 == 1:
            self.galvo1.frequency=value
            payload = self.galvo1.return_dict()
        else:
            self.galvo2.frequency=value
            payload = self.galvo2.return_dict()

        r = self.post_json(payload["task"], payload, timeout=1)
        return r

    def set_galvo_amp(self, axis=1, value=1000):
        if axis+1 == 1:
            self.galvo1.amplitude=value
            payload = self.galvo1.return_dict()
        else:
            self.galvo2.amplitude=value
            payload = self.galvo2.return_dict()

        r = self.post_json(payload["task"], payload, timeout=1)
        return r