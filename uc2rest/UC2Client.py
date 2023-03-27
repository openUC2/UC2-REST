#!/usr/bin/env python
# coding: utf-8
#%%
"""
Simple client code for the ESP32 in Python
Copyright 2021 Benedict Diederich, released under LGPL 3.0 or later
"""
from .mserial import Serial
try:
    from imswitch.imcommon.model import initLogger
    IS_IMSWITCH = True
except:
    print("No imswitch available")
    from .logger import Logger
    IS_IMSWITCH = False

import requests

class UC2Client(object):
    # headers = {'ESP32-version': '*'}
    headers={"Content-Type":"application/json"}
    getmessage = ""
    is_connected = False

    is_wifi = False
    is_serial = False

    # BAUDRATE = 500000
    BAUDRATE = 115200

    def __init__(self, host=None, port=31950, serialport=None, identity="UC2_Feather", baudrate=BAUDRATE, NLeds=64, DEBUG=False):
        '''
        This client connects to the UC2-REST microcontroller that can be found here
        https://github.com/openUC2/UC2-REST

        generally speaking you send/receive JSON documents that will cause an:
        1. action => "/XXX_act"
        2. getting => "/XXX_get"
        3. setting => "/XXX_set"

        you can send commands through wifi/http or usb/serial
        '''

        if IS_IMSWITCH:
            self.logger = initLogger(self, tryInheritParent=True)
        else:
            from .logger import Logger
            self.logger = Logger()
        # set default APIVersion
        self.APIVersion = 2

        # initialize communication channel (# connect to wifi or usb)
        if serialport is not None:
            # use USB connection
            self.serial = Serial(serialport, baudrate, parent=self, identity=identity, DEBUG=DEBUG)
            self.is_serial = True
            self.is_connected = True
            self.serial.DEBUG = DEBUG
        elif host is not None:
            # use client in wireless mode
            self.is_wifi = True
            self.host = host
            self.port = port

            # check if host is up
            self.logger.debug(f"Connecting to microscope {self.host}:{self.port}")
            self.is_connected = self.isConnected()
        else:
            self.logger.error("No ESP32 device is connected - check IP or Serial port!")

        # import libraries depending on API version
        if self.APIVersion == 1:
            self.logger.debug("Using API version 1")
            from .v1.galvo import Galvo
            from .v1.config import config
            from .v1.logger import Logger
            from .v1.ledmatrix import LedMatrix
            from .v1.motor import Motor
            from .v1.state import State
            from .v1.laser import Laser
            from .v1.wifi import Wifi
            from .v1.camera import Camera
            from .v1.analog import Analog
        elif self.APIVersion == 2:    
            self.logger.debug("Using API version 2")        
            from .galvo import Galvo
            from .config import config
            from .ledmatrix import LedMatrix
            from .motor import Motor
            from .home import Home
            from .state import State
            from .laser import Laser
            from .wifi import Wifi
            from .camera import Camera
            from .analog import Analog
            from .updater import updater
            from .modules import Modules
            from .digitalout import DigitalOut


        #FIXME
        #self.set_state(debug=False)

        # initialize state
        self.state = State(self)
        self.state.get_state()

        # initialize config
        self.config = config(self)

        # initialize LED matrix
        self.led = LedMatrix(self, NLeds=NLeds)

        # initilize motor
        self.motor = Motor(self)
        
        # initiliaze homing
        self.home = Home(self)

        # initialize laser
        self.state = State(self)

        # initialize galvo
        self.galvo = Galvo(self)

        # initialize laser
        self.laser = Laser(self)

        # initialize wifi
        self.wifi = Wifi(self)

        # initialize camera
        self.camera = Camera(self)

        # initialize analog
        self.analog = Analog(self)
        
        # initialize digital out
        self.digitalout = DigitalOut(self)
        
        # initialize config
        self.config = config(self)
        self.pinConfig = self.config.loadConfigDevice()
        
        # initialize updater 
        self.updater = updater(parent=self)
        
        # initialize module controller
        self.modules = Modules(parent=self)
   
    def post_json(self, path, payload, getReturn=True, timeout=1):
        if self.is_wifi:
            # FIXME: this is not working
            url = f"http://{self.host}:{self.port}{path}"
            r = requests.post(url, json=payload, headers=self.headers)
            return r.json()
        elif self.is_serial:
            return self.serial.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        else:
            self.logger.error("No ESP32 device is connected - check IP or Serial port!")
            return None

    def get_json(self, path, timeout=1):
        if self.is_wifi:
            # FIXME: this is not working
            url = f"http://{self.host}:{self.port}{path}"
            r = requests.get(url, headers=self.headers)
            return r.json()
        elif self.is_serial:
            self.serial.get_json(path)
            return self.serial.read_json()
        else:
            self.logger.error("No ESP32 device is connected - check IP or Serial port!")
            return None

    def setDebugging(self, debug=False):
        self.logger.debug(f"Setting debugging to {debug}")
        self.serial.DEBUG = debug