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
    default Configuration
    '''     
        
    def loadDefaultConfig(self):
        self.defaultConfig = {"motorconfig": 
                [{"stepperid": 0, "dir": 18, "step": 19, "enable": 12, "dir_inverted": False, "step_inverted": False, "enable_inverted": False, "speed": 0, "speedmax": 200000, "max_pos": 100000, "min_pos": -100000}, 
                {"stepperid": 1, "dir": 16, "step": 26, "enable": 12, "dir_inverted": False, "step_inverted": False, "enable_inverted": False, "speed": 0, "speedmax": 200000, "max_pos": 100000, "min_pos": -100000}, 
                {"stepperid": 2, "dir": 27, "step": 25, "enable": 12, "dir_inverted": False, "step_inverted": False, "enable_inverted": False, "speed": 0, "speedmax": 200000, "max_pos": 100000, "min_pos": -100000}, 
                {"stepperid": 3, "dir": 14, "step": 17, "enable": 12, "dir_inverted": False, "step_inverted": False, "enable_inverted": False, "speed": 0, "speedmax": 200000, "max_pos": 100000, "min_pos": -100000}], 
            "ledconfig": {"ledArrNum": 0, "ledArrPin": 0, "LEDArrMode": [0, 1, 2, 3, 4, 5, 6, 7], "led_ison": False}, 
            "stateconfig": {"identifier_name": "UC2_Feather", "identifier_id": "V1.2", "identifier_date": "Nov  7 202212:52:14", "identifier_author": "BD", "IDENTIFIER_NAME": ""}}            
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
        # load current configuration from device 
        if 0:
            path = '/config_get'
            payload = {
            }
            r = self._parent.post_json(path, payload, timeout=timeout)
            
            self.setDefaultConfig(r)
            
            if type(r) != dict:
                r = self.loadDefaultConfig()
            return r
        
        else:
            motorconfig = self._parent.motor.get_motors()
            ledconfig = self._parent.led.get_ledpin()
            laserconfig = self._parent.laser.get_laserpins()
            stateconfig = self._parent.state.get_state()
            self.configfile = {"motorconfig": motorconfig, 
                          "ledconfig": ledconfig, 
                          "laserconfig": laserconfig, 
                          "stateconfig": stateconfig}
            return self.configfile
            

    def setConfigDevice(self, configfile, timeout=1):
        # push changes back to device
        if 0:
            path = '/config_set'
            if type(configfile)==dict:
                payload = configfile
            else: 
                return None
            r = self._parent.post_json(path, payload, timeout=timeout)
            return r
        else: 
            # set all lasers 
            self._parent.laser.set_laserpin(laserid=1, laserpin=configfile["laserconfig"]["LASER1pin"])
            self._parent.laser.set_laserpin(laserid=2, laserpin=configfile["laserconfig"]["LASER2pin"])
            self._parent.laser.set_laserpin(laserid=3, laserpin=configfile["laserconfig"]["LASER3pin"])
            # set led 
            self._parent.led.set_ledpin(ledArrPin=configfile["ledconfig"]["ledArrPin"], ledArrNum=configfile["ledconfig"]["ledArrNum"])
            # set motors
            
            motorconfig = {
                "motor":{
                    "steppers": 
                        configfile["motorconfig"]}}
            self._parent.motor.set_motors(settingsdict=motorconfig)
            

