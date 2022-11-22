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
            "stateconfig": {"identifier_name": "UC2_Feather", "identifier_id": "V1.2", "identifier_date": "Nov  7 202212:52:14", "identifier_author": "BD", "IDENTIFIER_NAME": ""},
            "laserconfig": {"LASER1pin": 0, "LASER2pin": 0, "LASER3pin": 0}}            
        return self.defaultConfig
    
    def setDefaultConfig(self, configDict=None, configFile=None):
        if configDict is not None:
            self.defaultConfig = configDict
        if configFile is not None:
            self.defaultConfig = self.json2dict(configFile)
    
    
    def checkIfInitializedConfig(self, currentConfig, defaultConfig):
        ''' This should check if the different components were initialized with the config file
            Otherwise we won't be able to retrieve any signals from the device
        '''
        # TODO: Not implemented yet! 
        if currentConfig['motorconfig'] == defaultConfig['motorconfig']:
            print("motorconfig not initialized")
            return False

    def json2dict(self, jsonFile):
        with open(jsonFile) as f:
            data = json.load(f)
        return data
        
    def checkConfig(self, configfile):
        # check if config is valid
        if type(configfile['motorconfig'])==list and len(configfile['motorconfig'])==4:
            print("loaded config is valid")
            return True
        else:
            print("config is not valid")
            return False
        
        
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
                        
            if type(r) != dict:
                r = self.loadDefaultConfig()
            else:
                self.setDefaultConfig(r)
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

            # check if config is valid
            isValidConfig = self.checkConfig(self.configfile)

            if isValidConfig:
                return self.configfile
            else:
                return self.loadDefaultConfig()

            

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
            notAllowedPins = (1,2,3,5)
            if configfile["laserconfig"]["LASER1pin"] in notAllowedPins:
                self._parent.laser.set_laserpin(1, configfile["laserconfig"]["LASER1pin"])
            if configfile["laserconfig"]["LASER2pin"] in notAllowedPins:
                self._parent.laser.set_laserpin(2, configfile["laserconfig"]["LASER2pin"])
            if configfile["laserconfig"]["LASER3pin"] in notAllowedPins:
                self._parent.laser.set_laserpin(3, configfile["laserconfig"]["LASER3pin"])
            self._parent.laser.set_laserpin(laserid=1, laserpin=configfile["laserconfig"]["LASER1pin"])
            self._parent.laser.set_laserpin(laserid=2, laserpin=configfile["laserconfig"]["LASER2pin"])
            self._parent.laser.set_laserpin(laserid=3, laserpin=configfile["laserconfig"]["LASER3pin"])

            # set led 
            if configfile["ledconfig"]["ledArrPin"] in notAllowedPins:
                self._parent.led.set_ledpin(ledArrPin=configfile["ledconfig"]["ledArrPin"], ledArrNum=configfile["ledconfig"]["ledArrNum"])
            self._parent.led.set_ledpin(ledArrPin=configfile["ledconfig"]["ledArrPin"], ledArrNum=configfile["ledconfig"]["ledArrNum"])
            
                
            # set motors
            if configfile["motorconfig"][0]["dir"] in notAllowedPins:
                configfile["motorconfig"][0]["dir"] = 0
            if configfile["motorconfig"][0]["step"] in notAllowedPins:
                configfile["motorconfig"][0]["step"] = 0
            if configfile["motorconfig"][1]["dir"] in notAllowedPins:
                configfile["motorconfig"][1]["dir"] = 0
            if configfile["motorconfig"][1]["step"] in notAllowedPins:
                configfile["motorconfig"][1]["step"] = 0
            if configfile["motorconfig"][2]["dir"] in notAllowedPins:
                configfile["motorconfig"][2]["dir"] = 0
            if configfile["motorconfig"][2]["step"] in notAllowedPins:
                configfile["motorconfig"][2]["step"] = 0
            if configfile["motorconfig"][3]["dir"] in notAllowedPins:
                configfile["motorconfig"][3]["dir"] = 0
            if configfile["motorconfig"][3]["step"] in notAllowedPins:
                configfile["motorconfig"][3]["step"] = 0   
            if configfile["motorconfig"][0]["enable"] in notAllowedPins:
                configfile["motorconfig"][0]["enable"] = 0
                configfile["motorconfig"][1]["enable"] = 0
                configfile["motorconfig"][2]["enable"] = 0
                configfile["motorconfig"][3]["enable"] = 0
                                                         
            motorconfig = {
                "motor":{
                    "steppers": 
                        configfile["motorconfig"]}}
            # check if pins are valid
            
            self._parent.motor.set_motors(settingsdict=motorconfig)
            

