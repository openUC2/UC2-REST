import json


class config(object):
    def __init__(self, parent, configFilePath=None): 
        self._parent = parent
        self.configFilePath = configFilePath
        
        if self.configFilePath is not None:
            self.configFilePath = "./config.json"
            #TODO: read file and load
            
        self.configFile = self.loadDefaultConfig()
        
    
    '''
    Set Configuration
    '''
    
    def setMotorConfig(self, motXstp, motXdir, motYstp, motYdir, motZstp, motZdir, motAstp, motAdir, motEnable):
        self.configFile["motXstp"] = motXstp
        self.configFile["motXdir"] = motXdir
        self.configFile["motYstp"] = motYstp
        self.configFile["motYdir"] = motYdir
        self.configFile["motZstp"] = motZstp
        self.configFile["motZdir"] = motZdir
        self.configFile["motAstp"] = motAstp
        self.configFile["motAdir"] = motAdir
        self.configFile["motEnable"] = motEnable
        
    def setDigitalPin(self, pin, value):
        self.configFile["digitalPin" + str(pin)] = value
        
    def setAnalogPin(self, pin, value):
        self.configFile["analogPin" + str(pin)] = value
        
    def setLEDArrayConfig(self, ledArrPin, ledArrNum):
        configFile = {}
        configFile["ledArrPin"] = ledArrPin
        configFile["ledArrNum"] = ledArrNum
        self.setConfigDevice(configFile, timeout=1)
        
    def setLaserPinConfig(self, pin1, pin2, pin3):
        self.configFile["laserPin1"] = pin1
        self.configFile["laserPin2"] = pin2
        self.configFile["laserPin3"] = pin3
        
    def setDACFakeConfig(self, pin1, pin2):
        self.configFile["dacFake1"] = pin1
        self.configFile["dacFake2"] = pin2
        
    def setWIFIconfig(self, ssid, PW):
        self.configFile["ssid"] = ssid
        self.configFile["PW"] = PW
        
    def setIdentifier(self, identifier):
        self.configFile["identifier"] = identifier
        
    '''
    get Configuration
    '''
    def getMotorPinConfig(self):
        return [self.configFile["motXstp"], 
                self.configFile["motXdir"], 
                self.configFile["motYstp"], 
                self.configFile["motYdir"], 
                self.configFile["motZstp"], 
                self.configFile["motZdir"], 
                self.configFile["motAstp"], 
                self.configFile["motAdir"], 
                self.configFile["motEnable"]]
    
    def getLaserPinConfig(self):
        return [self.configFile["laserPin1"], self.configFile["laserPin2"], self.configFile["laserPin3"]]
    
    def getLEDArrayConfig(self):
        return [self.configFile["ledArrPin"], self.configFile["ledArrNum"]]
    
    def getWiFiConfig(self):
        return [self.configFile["ssid"], self.configFile["PW"]]
    
    def getIdentifier(self):
        return self.configFile["identifier"]
    
    def getAnalogPinConfig(self):
        return [self.configFile["analogPin1"], self.configFile["analogPin2"], self.configFile["analogPin3"]]
    
    def getDigitalPin(self, pin):
        return self.configFile["digitalPin" + str(pin)]
    
    '''
    default Configuration
    '''     
        
    def loadDefaultConfig(self):
        self.defaultConfig = {
            "motXstp": 0,
            "motXdir": 0,
            "motYstp": 0,
            "motYdir": 0,
            "motZstp": 0,
            "motZdir": 0,
            "motAstp": 0,
            "motAdir": 0,
            "motEnable": 0,
            "ledArrPin": 0,
            "ledArrNum": 64,
            "digitalPin1":0,
            "digitalPin2":0,
            "analogPin1":0,
            "analogPin2":0,
            "analogPin3":0,
            "laserPin1":0,
            "laserPin2":0,
            "laserPin3":0,
            "dacFake1":0,
            "dacFake2":0,
            "identifier": "TEST",
            "ssid": "ssid",
            "PW": "PW"
        }
        return self.defaultConfig
    
    def setDefaultConfig(self, configDict=None, configFile=None):
        if configDict is not None:
            self.defaultConfig = configDict
        if configFile is not None:
            self.defaultConfig = self.json2dict(configFile)
    
    # importing the module


    def json2dict(self, jsonFile):
        with open(jsonFile) as f:
            data = json.load(f)
        return data
        
    
    '''
    send configuration to device
    '''
    
    '''################################################################################################################################################
    Set Configurations
    ################################################################################################################################################'''

    def loadConfigDevice(self, timeout=1):
        path = '/config_get'
        payload = {
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        
        self.setDefaultConfig(r)
        
        if type(r) != dict:
            r = self.loadDefaultConfig()
        return r

    def setConfigDevice(self, config, timeout=1):
        path = '/config_set'
        if type(config)==dict:
            payload = config
        else: 
            return None
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r


    