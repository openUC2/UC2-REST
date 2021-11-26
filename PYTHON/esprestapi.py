#!/usr/bin/env python
# coding: utf-8
#%%
"""
Simple client code for the ESP32 in Python - adapted from OFM Client 
Copyright 2020 Richard Bowman, released under LGPL 3.0 or later
Copyright 2021 Benedict Diederich, released under LGPL 3.0 or later
"""

import requests
import json
import time
import io
#import PIL.Image
import numpy as np
import logging
import zeroconf
import requests 
import json
import time
import cv2
from tempfile import NamedTemporaryFile 
#import matplotlib.pyplot as plt

import apidef

ACTION_RUNNING_KEYWORDS = ["idle", "pending", "running"]
ACTION_OUTPUT_KEYS = ["output", "return"]

class ESP32Client(object):
    # headers = {'ESP32-version': '*'}
    headers={"Content-Type":"application/json"}

    api = apidef.apidef()

    def __init__(self, host, port=31950):
        if isinstance(host, zeroconf.ServiceInfo):
            # If we have an mDNS ServiceInfo object, try each address
            # in turn, to see if it works (sometimes you get addresses
            # that don't work, if your network config is odd).
            # TODO: figure out why we can get mDNS packets even when
            # the microscope is unreachable by that IP
            for addr in host.parsed_addresses():
                if ":" in addr:
                    self.host = f"[{addr}]"
                else:
                    self.host = addr
                self.port = host.port
                try:
                    self.get_json(self.base_uri)
                    break
                except:
                    logging.info(f"Couldn't connect to {addr}, we'll try another address if possible.")
        else:
            self.host = host
            self.port = port
            #self.get_json(self.base_uri)
        logging.info(f"Connecting to microscope {self.host}:{self.port}")
        #self.populate_extensions()

    is_filter_init = False
    filter_position = 0
        
    @property
    def base_uri(self):
        return f"http://{self.host}:{self.port}"

    def get_json(self, path):
        """Perform an HTTP GET request and return the JSON response"""
        if not path.startswith("http"):
            path = self.base_uri + path
        r = requests.get(path)
        r.raise_for_status()
        return r.json()

    def post_json(self, path, payload={}, headers=None, timeout=10):
        """Make an HTTP POST request and return the JSON response"""
        if not path.startswith("http"):
            path = self.base_uri + path
        if headers is None:
            headers = self.headers
            
        r = requests.post(path, json=payload, headers=headers,timeout=timeout)
        r.raise_for_status()
        r = r.json()
        return r
    
    #% LED
    def set_led(self, colour=(0,0,0)):
        payload = {
            "red": colour[0], 
            "green": colour[1], 
            "blue": colour[2]
        }
        path = self.api.PATH_LED
        r = self.post_json(path, payload)
        return r
    
    def set_laser(self, value):
        payload = {
            "value": value
        }
        path = self.api.PATH_LASER
        r = self.post_json(path, payload)
        return r    

    def set_laser_red(self, value, auto_filterswitch=False):
        if auto_filterswitch and value >0:
            self.switch_filter("R")
        payload = {
            "value": value
        }
        path = self.api.PATH_LASER_RED
        r = self.post_json(path, payload)
        return r    
    
    def set_laser_green(self, value, auto_filterswitch=False):
        if auto_filterswitch and value >0:
            self.switch_filter("G")
        payload = {
            "value": value
        }
        path = self.api.PATH_LASER_GREEN
        r = self.post_json(path, payload)
        return r    
    
    def set_laser_blue(self, value, auto_filterswitch=False):
        if auto_filterswitch and value >0:
            self.switch_filter("B")
        payload = {
            "value": value
        }
        path = self.api.PATH_LASER_BLUE
        r = self.post_json(path, payload)
        return r    
    
    def move_x(self, steps=100, speed=10):
        payload = {
            "steps": steps, 
            "speed": speed,            
        }
        path = self.api.PATH_MOVE_X
        r = self.post_json(path, payload)
        return r
    
    def move_y(self, steps=100, speed=10):
        payload = {
            "steps": steps, 
            "speed": speed,            
        }
        path = self.api.PATH_MOVE_Y
        r = self.post_json(path, payload)
        return r
    
    def move_z(self, steps=100, speed=10,timeout=1):
        payload = {
            "steps": steps, 
            "speed": speed,            
        }
        path = self.api.PATH_MOVE_Z
        r = self.post_json(path, payload,timeout=timeout)
        return r

    def move_filter(self, steps=100, speed=10,timeout=1):
        payload = {
            "steps": steps, 
            "speed": speed,            
        }
        path = self.api.PATH_FILTER_MOVE
        r = self.post_json(path, payload,timeout=timeout)
        return r

    def lens_x(self, value=100):
        payload = {
            "lens_value": value,            
        }
        path = self.api.PATH_LENS_X
        r = self.post_json(path, payload)
        return r
    
    def lens_z(self, value=100):
        payload = {
            "lens_value": value,            
        }
        path = self.api.PATH_LENS_Z
        r = self.post_json(path, payload)
        return r
    
    def send_jpeg(self, image):
        temp = NamedTemporaryFile()
        
        #add JPEG format to the NamedTemporaryFile  
        iName = "".join([str(temp.name),".jpg"])
        
        #save the numpy array image onto the NamedTemporaryFile
        cv2.imwrite(iName,image)
        # TODO: not ideal _, img_encodedsv2.imencode('test.jpg', image)
        path = self.api.PATH_LCD_DISP
        files = {'media': open(iName, 'rb')}
        requests.post(self.base_uri + path, files=files)
        
    def switch_filter(self, colour="R", timeout=20, is_filter_init=None, speed=20):
        
        # switch off all lasers first!        
        self.set_laser_red(0)
        self.set_laser_blue(0)
        self.set_laser_green(0)

        if is_filter_init is not None:
            self.is_filter_init = is_filter_init
            
        if not self.is_filter_init:
            self.move_filter(steps=-2000, speed=speed)
            time.sleep(4)
            self.is_filter_init = True
            self.filter_position = 0
            
        # measured in steps from zero position
        pos_blue = 300
        pos_green = 900
        pos_red = 1500
            
        steps = 0
        if colour=="R":
            steps = pos_red-self.filter_position
            self.filter_position = pos_red
        if colour=="G":
            steps = pos_green - self.filter_position
            self.filter_position = pos_green
        if colour=="B":
            steps = pos_blue - self.filter_position
            self.filter_position = pos_blue
            
        self.move_filter(steps=steps, speed=speed)
        
    def send_ledmatrix(self, led_pattern):
        headers = {"Content-Type":"application/json"}
        path = self.api.PATH_LED_MATRIX
        payload = {
            "red": led_pattern[0,:,:].flatten().tolist(), 
            "green": led_pattern[1,:,:].flatten().tolist(),            
            "blue": led_pattern[2,:,:].flatten().tolist()                 
        }
        requests.post(self.base_uri + path, data=json.dumps(payload), headers=headers)
        return None
           
    def set_galvo_amplitudex(self, amplitudex = 0, timeout=1):
        payload = {
            "value": amplitudex,           
        }
        path = self.api.PATH_GALVO_AMP_X
        r = self.post_json(path, payload,timeout=timeout)
        return r

