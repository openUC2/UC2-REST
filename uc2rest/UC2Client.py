#!/usr/bin/env python
# coding: utf-8
#%%
"""
Simple client code for the ESP32 in Python
Copyright 2021 Benedict Diederich, released under LGPL 3.0 or later
"""

#import numpy as np



from .galvo import Galvo
from .config import config
from .logger import Logger
from .mserial import Serial
from .ledmatrix import LedMatrix
from .motor import Motor
from .state import State
from .laser import Laser
from .wifi import Wifi
from .camera import Camera


try:
    from imswitch.imcommon.model import initLogger
    IS_IMSWITCH = True
except:
    print("No imswitch available")
    IS_IMSWITCH = False


class UC2Client(object):
    # headers = {'ESP32-version': '*'}
    headers={"Content-Type":"application/json"}
    getmessage = ""
    is_connected = False

    is_wifi = False
    is_serial = False

    BAUDRATE = 115200
    
    def __init__(self, host=None, port=31950, serialport=None, identity=None, baudrate=BAUDRATE):
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
            self.logger = Logger()            

        # initialize communication channel (# connect to wifi or usb)
        if serialport is not None:
            # use USB connection
            self.serial = Serial(serialport, baudrate, parent=self, identity=identity)
            self.is_serial = True
            self.is_connected = True
            
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

        #FIXME
        #self.set_state(debug=False)

        # initialize galvos
        #self.galvo1 = Galvo(channel=1) FIXME
        #self.galvo2 = Galvo(channel=2) FIXME
        
        # initialize config
        self.config = config(self)
        self.pinConfig = self.config.loadConfigDevice()
        
        # initialize LED matrix
        try: NLeds = self.pinConfig["ledArrNum"]
        except: NLeds=64
        self.led = LedMatrix(self, NLeds=NLeds)
        
        # initilize motor
        self.motor = Motor(self)
        
        # initialize laser
        self.state = State(self)
        
        # initialize galvo
        self.galvo1 = Galvo(self, 1)
        
        # initialize laser
        self.laser = Laser(self)
        
        # initialize wifi
        self.wifi = Wifi(self)
        
        # initialize camera
        self.camera = Camera(self)

    def post_json(self, path, payload, timeout=1):
        if self.is_wifi:
            # FIXME: this is not working
            url = f"http://{self.host}:{self.port}{path}"
            r = requests.post(url, json=payload, headers=self.headers)
            return r.json()
        elif self.is_serial:
            return self.serial.post_json(path, payload, timeout=timeout)
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
            self.serial.get_json(self, path)
            return self.serial.read_json()
        else:
            self.logger.error("No ESP32 device is connected - check IP or Serial port!")
            return None










