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
  
    def switch_filter(self, value=100):
        payload = {
            "None": 1,            
        }
        path = '/switch_filter'
        r = self.post_json(path)
        return r
    
    def send_jpeg(self, image):

        temp = NamedTemporaryFile()
        
        #add JPEG format to the NamedTemporaryFile  
        iName = "".join([str(temp.name),".jpg"])
        
        #save the numpy array image onto the NamedTemporaryFile
        cv2.imwrite(iName,image)
        _, img_encoded = cv2.imencode('test.jpg', image)

        content_type = 'image/jpeg'
        headers = {'content-type': content_type}
        payload = img_encoded.tostring()
        path = '/uploadimage'

        #r = self.post_json(path, payload=payload, headers = headers)
        #requests.post(self.base_uri + path, data=img_encoded.tostring(), headers=headers)      
        files = {'media': open(iName, 'rb')}
        requests.post(self.base_uri + path, files=files)
        
    def switch_filter(self, timeout=20):
        payload = {
            "None": 0,            
        }
        path = '/switch_filter'
        r = self.post_json(path, payload, timeout=timeout)
        self.set_laser_red(0)
        self.set_laser_blue(0)
        self.set_laser_green(0)
        return r
             
        
    def send_ledmatrix(self, led_pattern):
        headers = {"Content-Type":"application/json"}
        path = '/matrix'
        payload = {
            "red": led_pattern[0,:,:].flatten().tolist(), 
            "green": led_pattern[1,:,:].flatten().tolist(),            
            "blue": led_pattern[2,:,:].flatten().tolist()                 
        }
        print(self.base_uri + path)
        print(payload)
        requests.post(self.base_uri + path, data=json.dumps(payload), headers=headers)
        #r = self.post_json(path, payload=payload, headers = headers)
        
           
    def move_filter(self, steps=100, speed=10,timeout=1):
        payload = {
            "steps": steps, 
            "speed": speed,            
        }
        path = '/move_filter'
        r = self.post_json(path, payload,timeout=timeout)
        return r
    
        
        
    
#%
host = '192.168.43.226'
host = '192.168.2.147'
host = '192.168.2.151'
esp32 = ESP32Client(host, port=80)

#esp32.set_led((100,2,5))
#esp32.move_x(steps=2000, speed=8)
#esp32.move_y(steps=2000, speed=6)

#%%
esp32.lens_x(value=10000)
esp32.lens_z(value=10000)
#%%
for ix in range(0,32000,100):
    esp32.lens_x(value=ix)
    for iy in range(0,32000,100):
        esp32.lens_z(value=iy)
esp32.lens_z(value=0)
esp32.lens_x(value=0)

#%%
esp32.lens_x(value=0)
esp32.lens_z(value=0)
#%%
for iy in range(0,1000,1):
    esp32.set_laser(np.sin(iy/1000*np.pi*100)*10000)

    
#%%
esp32.move_z(steps=500,  speed=1000)

#%%
for i in range(100):
    print(i)
    esp32.move_z(steps=i*100, speed=i*100)
    

#%%
for i in range(100):
    print(i)
    esp32.post(value = i)

#%%
esp32.set_laser_red(10000)
esp32.set_laser_blue(10000)
esp32.set_laser_green(10000)

time.sleep(1)

esp32.set_laser_red(0)
esp32.set_laser_blue(0)
esp32.set_laser_green(0)
#%%
esp32.move_filter(steps=-800, speed=20)
# %%
esp32.set_laser_red(0)
esp32.set_laser_blue(0000)

#%%
esp32.set_laser_red(0)

#%%
esp32.set_led(colour=(0,255,255))

#%%
esp32.switch_filter()

#%%
image = np.random.randn(320,240)*255
esp32.send_jpeg(image)

#%%
N_leds = 4
I_max = 100
iiter = 0 
while(True):
    iiter+=1
    
    image = np.ones((320,240))*(iiter%2)*255 # np.random.randn(320,240)*
    esp32.send_jpeg(np.uint8(image))
    led_pattern = np.array((np.reshape(np.random.randint(0,I_max ,N_leds**2),(N_leds,N_leds)),
                   np.reshape(np.random.randint(0,I_max ,N_leds**2),(N_leds,N_leds)),
                   np.reshape(np.random.randint(0,I_max ,N_leds**2),(N_leds,N_leds))))
    
    esp32.send_ledmatrix(led_pattern)