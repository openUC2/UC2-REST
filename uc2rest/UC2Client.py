#!/usr/bin/env python
# coding: utf-8
#%%
"""
Simple client code for the ESP32 in Python
Copyright 2021 Benedict Diederich, released under LGPL 3.0 or later
"""
from .mserial import Serial
from .mserial import SerialManagerWrapper
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
from .modules import Modules
from .digitalout import DigitalOut
from .rotator import Rotator
from .logger import Logger
from .cmdrecorder import cmdRecorder

try:
    import requests
except:
    print("No requests available - running on pyscript?")

class UC2Client(object):
    # headers = {'ESP32-version': '*'}
    headers={"Content-Type":"application/json"}
    getmessage = ""
    is_connected = False

    is_wifi = False
    is_serial = False

    # BAUDRATE = 500000
    BAUDRATE = 115200

    def __init__(self, host=None, port=31950, serialport=None, identity="UC2_Feather", baudrate=BAUDRATE, NLeds=64, SerialManager=None, DEBUG=False, logger=None):
        '''
        This client connects to the UC2-REST microcontroller that can be found here
        https://github.com/openUC2/UC2-REST

        generally speaking you send/receive JSON documents that will cause an:
        1. action => "/XXX_act"
        2. getting => "/XXX_get"
        3. setting => "/XXX_set"

        you can send commands through wifi/http or usb/serial
        '''
        if logger is None:
            self.logger = Logger()
        else:
            self.logger = logger

        # perhaps we are in the browser?
        self.isPyScript = False

        # initialize communication channel (# connect to wifi or usb)
        if serialport is not None:
            # use USB connection
            self.serial = Serial(serialport, baudrate, parent=self, identity=identity, DEBUG=DEBUG)
            self.is_serial = True
            self.is_connected = self.serial.is_connected
            self.serial.DEBUG = DEBUG
        elif host is not None:
            # use client in wireless mode
            self.is_wifi = True
            self.host = host
            self.port = port

            # check if host is up
            self.logger.debug(f"Connecting to microscope {self.host}:{self.port}")
            #self.is_connected = self.isConnected()
        elif SerialManager is not None:
            # we are trying to access the controller from .a web browser
            self.serial = SerialManagerWrapper(SerialManager, parent=self)
            self.isPyScript = True
        else:
            self.logger.error("No ESP32 device is connected - check IP or Serial port!")
        
                
        if not self.isPyScript: from .updater import updater
        
        # import libraries depending on API version
        self.logger.debug("Using API version 2")        

        #FIXME
        #self.set_state(debug=False)

        # initialize state
        self.state = State(self)
        if not self.isPyScript: 
            state = self.state.get_state()
       
        # initialize config
        if not self.isPyScript: 
            self.config = config(self)

        # initialize cmdRecorder
        self.cmdRecorder = cmdRecorder(self)
        
        # initialize LED matrix
        self.led = LedMatrix(self, NLeds=NLeds)

        # initilize motor
        self.motor = Motor(self)
        
        # initialize rotator
        self.rotator = Rotator(self)
        
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
        if False: # not self.isPyScript: 
            self.config = config(self)
            try: self.pinConfig = self.config.loadConfigDevice()
            except: self.pinConfig = None
        
        # initialize updater 
        if not self.isPyScript: 
            try: self.updater = updater(parent=self)
            except: self.updater = None
        
        # initialize module controller
        self.modules = Modules(parent=self)
    
    def post_json(self, path, payload, getReturn=True, nResponses=1, timeout=1):
        if self.is_wifi:
            # FIXME: this is not working
            url = f"http://{self.host}:{self.port}{path}"
            try:
                r = requests.post(url, json=payload, headers=self.headers,  timeout=timeout)
                returnMessage = r.json()
                returnMessage["success"] = r.status_code==200
            except Exception as e:
                print(e)
                returnMessage = {}
                returnMessage["error"] = str(e)
                returnMessage["success"] = 0
            return returnMessage
        elif self.is_serial or self.isPyScript:
            if timeout <=0:
                getReturn = False
            return self.serial.post_json(path, payload, getReturn=getReturn, nResponses=nResponses)
        else:
            self.logger.error("No ESP32 device is connected - check IP or Serial port!")
            return None

    def get_json(self, path, getReturn=True, timeout=1):
        if self.is_wifi:
            # FIXME: this is not working
            url = f"http://{self.host}:{self.port}{path}"
            r = requests.get(url, headers=self.headers, timeout=timeout)
            return r.json()
        elif self.is_serial or self.isPyScript:
            # timeout is not used anymore
            if timeout <=0:
                getReturn = False
            return self.serial.post_json(path, payload=None, getReturn=getReturn, nResponses=1)
            #return self.serial.read_json()
        else:
            self.logger.error("No ESP32 device is connected - check IP or Serial port!")
            return None

    def setDebugging(self, debug=False):
        self.logger.debug(f"Setting debugging to {debug}")
        self.serial.DEBUG = debug
        
    def close(self):
        self.serial.closeSerial()