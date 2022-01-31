#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Jan 31 07:16:26 2022

@author: bene
"""

from ESP32RestSerialAPI import ESP32Client

serialport = "/dev/cu.SLAB_USBtoUART"
serialport = "/dev/cu.SLAB_USBtoUART"
esp32 = ESP32Client(serialport=serialport)

#%%
r = esp32.move_stepper(axis=2, steps=100, speed=100, is_absolute=False, is_blocking=True)
print(r)

#%%
r = esp32.set_laser(channel='B', value=00000)
print(r)

#%%
payload = {"task":"/motor_get", "axis":2}
esp32.writeSerial(payload)
r = esp32.readSerial()
print(r)

#%%
payload = {"task":"/motor_act", "axis":2, "speed": 100, "position": 100, "isabsolute":1}
esp32.writeSerial(payload)
r = esp32.readSerial()
print(r)

#%%
payload = {"task":"/LASER_act", "LASERid": 1, "LASERval": 1000}
esp32.writeSerial(payload)
r = esp32.readSerial()
print(r)