#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Apr 20 10:35:26 2022

@author: bene
"""
'''
%load_ext autoreload
%autoreload 2
'''

from ESP32Client import ESP32Client  
import logging
import threading
import time
import matplotlib.pyplot as plt
import collections
import time
import numpy as np
#https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0175089

allMeasurements = []
currentMeasurement = 0


nBuffer = 100
TMeasure = .01
allMeasurements = collections.deque(maxlen=nBuffer)
is_measure = True


setPoint = 550
error = 0
errorRunSum = 0
previousError = 0
stepperOut = 1
maxError = 5.0
sensorOffset = 0
stepperMaxValue = 200

Kp = 32500;
Ki = 4;
Kd = 5000;


def returnControlValue(setPoint=1, sensorValue=1, Kp=1, Ki=1, Kd=1):
    error = (setPoint - (sensorValue-sensorOffset)) / maxError
    global errorRunSum 
    global previousError
    cP = Kp * error
    cI = Ki * errorRunSum
    cD = Kd * (error - previousError)
    PID = cP+cI+cD
    

    
    PID = cP + cI + cD
  
    stepperOut = PID

    if (stepperOut > stepperMaxValue):
        stepperOut = stepperMaxValue
        
    if (stepperOut < -stepperMaxValue):
        stepperOut = -stepperMaxValue
        
    errorRunSum = errorRunSum + error
    previousError = error
    
    #print(f"P{cP}, I{cI}, D{cD}, errorRunSum{errorRunSum}, previousError{previousError}, stepperOut{stepperOut}, PID{PID}")
    
    return stepperOut

def start_measurement_thread():
    measurement_thread = threading.Thread(target=measurement_grabber, args=())
    measurement_thread.start()
    measurement_thread.join()
    ESP32.move_forever(speed=(0,0,0), is_stop=False, timeout=1)

def measurement_grabber():
    while(True):
        if not is_measure:
            print("Stopping")
            break
        try:
            currentMeasurement = ESP32.read_sensor(sensorID=0, NAvg=10)+550
            if currentMeasurement is not None:
                #print(currentMeasurement)

                motorValue = returnControlValue(setPoint, currentMeasurement, Kp, Ki, Kd)
                ESP32.move_forever(speed=(motorValue,0,0), is_stop=False, timeout=1)
                allMeasurements.append((currentMeasurement,motorValue))                
            time.sleep(TMeasure)
        except Exception as e:
            print(e)
        
        # update plot
        '''
        plt.plot(np.array(allMeasurements)[:,0])
        plt.plot(np.array(allMeasurements)[:,1])
        plt.show()
        '''

ESP32 = ESP32Client(serialport="unknown")



start_measurement_thread()

