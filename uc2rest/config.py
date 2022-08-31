import json

class config(object):
    def __init__(self, configFilePath=None):
        
        self.configFilePath = configFilePath
        
        if self.configFilePath is not None:
            self.configFilePath = "./config.json"
            #TODO: read file and load
            
        self.configFile = self.getDummyConfig()
        
    
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
        self.configFile["ledArrPin"] = ledArrPin
        self.configFile["ledArrNum"] = ledArrNum
        
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
        
    def getDefaultConfig(self):
        self.defaultConfig = {
            "motXstp": 1,
            "motXdir": 2,
            "motYstp": 3,
            "motYdir": 4,
            "motZstp": 5,
            "motZdir": 6,
            "motAstp": 7,
            "motAdir": 8,
            "motEnable": 9,
            "ledArrPin": 0,
            "ledArrNum": 64,
            "digitalPin1":10,
            "digitalPin2":11,
            "analogPin1":12,
            "analogPin2":13,
            "analogPin3":14,
            "laserPin1":15,
            "laserPin2":16,
            "laserPin3":17,
            "dacFake1":18,
            "dacFake2":19,
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
    
    