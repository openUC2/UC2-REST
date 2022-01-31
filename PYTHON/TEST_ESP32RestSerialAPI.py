#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Jan 31 07:16:26 2022

@author: bene
"""

from ESP32RestSerialAPI import ESP32Client

serialport = "/dev/cu.SLAB_USBtoUART"
esp32 = ESP32Client(serialport=serialport)

esp32.move_stepper(axis=1, steps=100, speed=10, is_absolute=False, is_blocking=False)