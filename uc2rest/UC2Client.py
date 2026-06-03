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
from .ledmatrix import LedMatrix
from .lcddisplay import LCDDisplay
from .motor import Motor
from .gripper import Gripper
from .home import Home
from .objective import Objective
from .state import State
from .laser import Laser
from .wifi import Wifi
from .camera import Camera
from .analog import Analog
from .modules import Modules
from .digitalout import DigitalOut
from .digitalin import DigitalIn
from .rotator import Rotator
from .logger import Logger
from .cmdrecorder import cmdRecorder
from .temperature import Temperature
from .fan import Fan
from .message import Message
from .can import CAN
from .canota import CANOTA
from .camera_trigger import CameraTrigger

# requests is no longer used for direct ESP communication (serial-only).
# Kept as optional import for other modules that may need HTTP (e.g. firmware downloads).
try:
    import requests
except Exception:
    requests = None

class UC2Client(object):
    headers={"Content-Type":"application/json"}
    getmessage = ""
    is_connected = False

    is_serial = False
    BAUDRATE = 115200

    def __init__(self, host=None, port=31950, serialport=None, identity="UC2_Feather", baudrate=BAUDRATE, 
                 NLeds=64, SerialManager=None, DEBUG=False, logger=None, skipFirmwareCheck=False,
                 isPyScript=False):
        '''
        This client connects to the UC2-REST microcontroller that can be found here
        https://github.com/openUC2/UC2-REST

        Communication is via USB/serial only.
        The host/port parameters are deprecated and will be ignored.

        generally speaking you send/receive JSON documents that will cause an:
        1. action => "/XXX_act"
        2. getting => "/XXX_get"
        3. setting => "/XXX_set"
        '''
        if True: #logger is None:
            self.logger = Logger()
        else:
            self.logger = logger

        # perhaps we are in the browser?
        self.isPyScript = isPyScript

        # Deprecation notice for WiFi mode
        if host is not None and serialport is None and SerialManager is None:
            self.logger.warning(
                "WiFi/HTTP communication has been removed. "
                "Please use serialport= instead. Ignoring host parameter."
            )

        # initialize communication channel (serial only)
        if serialport is not None:
            # use USB connection
            self.serial = Serial(serialport, baudrate, parent=self, identity=identity, DEBUG=DEBUG, skipFirmwareCheck=skipFirmwareCheck)
            self.is_serial = True
            self.is_connected = self.serial.is_connected
            self.serial.DEBUG = DEBUG
        elif SerialManager is not None:
            # we are trying to access the controller from a web browser
            self.serial = SerialManagerWrapper(SerialManager, parent=self)
            self.isPyScript = True
            self.is_serial = True
        else:
            self.logger.error("No ESP32 device is connected - please provide a serialport!")
        
                        
        # import libraries depending on API version
        self.logger.debug("Using API version 2")        

        # initialize state
        self.state = State(self)

        # initialize cmdRecorder
        self.cmdRecorder = cmdRecorder(self)
        
        # initialize LED matrix
        self.led = LedMatrix(self, NLeds=NLeds)

        # initialize LCD display
        self.lcd = LCDDisplay(self)
        
        # initilize motor
        self.motor = Motor(self)
        
        # initialize CAN
        self.can = CAN(self)
        
        # initialize CAN OTA
        self.canota = CANOTA(self)
        
        # initialize gripper
        self.gripper = Gripper(self)
        
        # initialize rotator
        self.rotator = Rotator(self)
        
        # initiliaze homing
        self.home = Home(self)
        
        # initialize objective
        self.objective = Objective(self)
        
        # initialize temperature
        self.temperature = Temperature(self)

        # initialize fan controller
        self.fan = Fan(self)

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
        
        # initialize digital in
        self.digitalin = DigitalIn(self)
        
        # initialize messaging
        self.message = Message(self)
        
        # initialize camera trigger callback handler
        self.camera_trigger = CameraTrigger(self)

        # initialize module controller
        self.modules = Modules(parent=self)
    
    def post_json(self, path, payload, getReturn=True, nResponses=1, timeout=1):
        if timeout <=0:
            getReturn = False
        return self.serial.post_json(path, payload, getReturn=getReturn, nResponses=nResponses, timeout=timeout)

    def get_json(self, path, getReturn=True, timeout=1):
        if self.is_serial or self.isPyScript:
            if timeout <= 0:
                getReturn = False
            return self.serial.post_json(path, payload=None, getReturn=getReturn, nResponses=1, timeout=timeout)
        else:
            self.logger.error("No ESP32 device is connected - serial not initialized!")
            return None

    def setDebugging(self, debug=False):
        self.logger.debug(f"Setting debugging to {debug}")
        self.serial.DEBUG = debug
        
    def close(self):
        self.serial.close()