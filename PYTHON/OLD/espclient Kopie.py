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
#import matplotlib.pyplot as plt

ACTION_RUNNING_KEYWORDS = ["idle", "pending", "running"]
ACTION_OUTPUT_KEYS = ["output", "return"]

class ESP32Client(object):
    # headers = {'ESP32-version': '*'}
    headers={"Content-Type":"application/json"}

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

    extensions = None
        
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

    def post_json(self, path, payload={}, timeout=10):
        """Make an HTTP POST request and return the JSON response"""
        if not path.startswith("http"):
            path = self.base_uri + path

        r = requests.post(path, json=payload, headers=self.headers,timeout=timeout)
        r.raise_for_status()
        r = r.json()
        return r


    def get_temperature(self):
        path = "/temperature"
        r = self.get_json(path)
        return r['value']
    
    #% LED
    def set_led(self, colour=(0,0,0)):
        payload = {
            "red": colour[0], 
            "green": colour[1], 
            "blue": colour[2]
        }
        path = '/led'
        r = self.post_json(path, payload)
        return r
    
    def set_laser(self, value):
        payload = {
            "value": value
        }
        path = '/laser'
        r = self.post_json(path, payload)
        return r    

    def set_laser_red(self, value):
        payload = {
            "value": value
        }
        path = '/laser_red'
        r = self.post_json(path, payload)
        return r    
    
    def set_laser_green(self, value):
        payload = {
            "value": value
        }
        path = '/laser_green'
        r = self.post_json(path, payload)
        return r    
    
    def set_laser_blue(self, value):
        payload = {
            "value": value
        }
        path = '/laser_blue'
        r = self.post_json(path, payload)
        return r    
    
    
    def move_x(self, steps=100, speed=10):
        payload = {
            "steps": steps, 
            "speed": speed,            
        }
        path = '/move_x'
        r = self.post_json(path, payload)
        return r
    
    def move_y(self, steps=100, speed=10):
        payload = {
            "steps": steps, 
            "speed": speed,            
        }
        path = '/move_y'
        r = self.post_json(path, payload)
        return r
    
    def lens_x(self, value=100):
        payload = {
            "lens_value": value,            
        }
        path = '/lens_x'
        r = self.post_json(path, payload)
        return r

    def move_z(self, steps=100, speed=10,timeout=1):
        payload = {
            "steps": steps, 
            "speed": speed,            
        }
        path = '/move_z'
        r = self.post_json(path, payload,timeout=timeout)
        return r

    def move_filter(self, steps=100, speed=10,timeout=1):
        payload = {
            "steps": steps, 
            "speed": speed,            
        }
        path = '/move_filter'
        r = self.post_json(path, payload,timeout=timeout)
        return r
    
    def lens_z(self, value=100):
        payload = {
            "lens_value": value,            
        }
        path = '/lens_z'
        r = self.post_json(path, payload)
        return r
  
    def post(self, value=100):
        payload = {
            "lens_value": value,            
        }
        path = '/post'
        r = self.post_json(path, payload)
        return r
    