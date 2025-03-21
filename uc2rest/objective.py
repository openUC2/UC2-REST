import time
import json

class Objective(object):
    def __init__(self, parent):
        """
        parent: an object that handles the low-level post_json(path, payload, getReturn, timeout, nResponses)
        similar to the 'Home' class example.
        """
        self._parent = parent
        # Default parameters (adjust as needed)
        self.speed = 20000
        self.accel = 20000
        self.direction = -1
        self.endstoppolarity = 1
        self.timeout = 10
        # Default objective positions
        self.x1 = 1000
        self.x2 = 2000
        self.z1 = 0
        self.z2 = 0
        self.objectiveAxis = 0

        
    def home(self, direction=None, endstoppolarity=None, speed=None, acceleration=None, hometimeout=20000, axis=None, isBlocking=False):
        # {"task":"/home_act", "home": {"steppers": [{"stepperid":0, "timeout": 20000, "speed": 10000, "direction":-1, "endstoppolarity":1}]}}
        if axis is None:
            axis = self.objectiveAxis
        if endstoppolarity is None:
            endstoppolarity = self.endstoppolarity
        if direction is None:
            direction = self.direction
        if speed is None:
            speed = self.speed
        if acceleration is None:
            acceleration = self.accel
        path = "/home_act"
        payload = {
            "task": path,
            "home": {
                "steppers": [
                    {
                        "stepperid": axis,
                        "timeout": hometimeout,
                        "speed": speed,
                        "direction": direction,
                        "endstoppolarity": endstoppolarity, 
                    }
                ]
            }
        }
        nResponses = 2 if isBlocking else 0
        r = self._parent.post_json(path, payload,
                                        getReturn=isBlocking, 
                                        timeout=self.timeout if isBlocking else 0, 
                                        nResponses=nResponses)
        return r
        
    def setX1(self, x1):
        self.x1 = x1
        
    def setX2(self, x2):
        self.x2 = x2
        
    def setZ1(self, z1):
        self.z1 = z1
    
    def setZ2(self, z2):
        self.z2 = z2
        
    def getstatus(self):
        """
        Get the objective status.
        returns:  
        {"objective":{"x1":1000,"x2":2000,"z1":0, "z2":0, "pos":1000,"isHomed":1,"state":1,"isRunning":0}}
        """
        path = "/objective_get"
        payload = {
            "task": path,
        }
        r = self._parent.post_json(path, payload, timeout=1)
        # parse the response
        try:
            status = r[-1]["objective"]
            status["x1"] = np.round(status["x1"]*self._parent.motor.stepSizeA)
            status["x2"] = np.round(status["x2"]*self._parent.motor.stepSizeA)
            status["z1"] = np.round(status["z1"]*self._parent.motor.stepSizeZ)
            status["z2"] = np.round(status["z2"]*self._parent.motor.stepSizeZ)
        except:
            status = {"x1":0, 
                      "x2":0, 
                      "z1":0,
                      "z2":0,
                      "pos":0, 
                      "isHomed":0, 
                      "state":0, 
                      "isRunning":0}
        return status
        
    def calibrate(self, direction=None, endstoppolarity=None, isBlocking=False):
        """
        Calibrate the objective (sets the position=0 position).
        direction => homing direction (e.g. 1 or -1)
        endstoppolarity => polarity for endstop (e.g. 1 or -1)
        isBlocking => if True, waits for the MCU's response (home done); else returns immediately
        """
        if direction is None:
            direction = self.direction
        if endstoppolarity is None:
            endstoppolarity = self.endstoppolarity

        path = "/objective_act"
        payload = {
            "task": path,
            "calibrate": 1,
            "homeDirection": direction,
            "homeEndStopPolarity": endstoppolarity
        }

        # if blocking, wait for two responses: "ok" + "home done"
        nResponses = 2 if isBlocking else 0 #
        r = self._parent.post_json(
            path, 
            payload, 
            getReturn=isBlocking, 
            timeout=self.timeout if isBlocking else 0, 
            nResponses=nResponses
        )
        return r

    def toggle(self, speed=None, accel=None, isBlocking=False):
        """
        Toggle between x1 and x2.
        speed, accel => override default speeds
        """
        if speed is None:
            speed = self.speed
        if accel is None:
            accel = self.accel

        path = "/objective_act"
        payload = {
            "task": path,
            "toggle": 1,
            "speed": abs(speed),
            "accel": abs(accel)
        }

        # if blocking, wait for two responses: "ok" + "motor done"
        nResponses = 2 if isBlocking else 0
        r = self._parent.post_json(
            path, 
            payload, 
            getReturn=isBlocking, 
            timeout=self.timeout if isBlocking else 0, 
            nResponses=nResponses
        )
        return r

    def move(self, slot=1, speed=None, accel=None, isBlocking=False):
        """
        Move to slot 1 (x1/z1) or slot 2 (x2/z2).
        slot => 1 or 2
        speed, accel => override default speeds
        """
        if speed is None:
            speed = self.speed
        if accel is None:
            accel = self.accel

        path = "/objective_act"
        payload = {
            "task": path,
            "move": slot,
            "speed": abs(speed),
            "accel": abs(accel),
            "obj": slot  # optional if you want to include it exactly as shown
        }

        nResponses = 2 if isBlocking else 0
        r = self._parent.post_json(
            path, 
            payload, 
            getReturn=isBlocking, 
            timeout=self.timeout if isBlocking else 0, 
            nResponses=nResponses
        )
        return r

    def setPositions(self, x1=None, x2=None, z1=None, z2=None, isBlocking=False):
        """
        Set explicit positions in steps.
        If x1 == -1, the current position is used as x1.
        Example:
            {"task":"/objective_act", "x1": 1000, "x2": 2000}
            {"task":"/objective_act", "x1": -1, "x2": 2000}
            {"task":"/objective_act", "x1": 1000, "x2": -1, "z1": 1, "z2": 100}
        """
        if x1 is not None:
            self.x1 = x1
        if x2 is not None:
            self.x2 = x2
        if z1 is not None:
            self.z1 = z1
        if z2 is not None:
            self.z2 = z2

        path = "/objective_act"
        payload = {"task": path}

        # Only include x1 or x2 in JSON if they were explicitly provided
        # If x1 == -1, that instructs the MCU to take current motor position
        if x1 is not None:
            payload["x1"] = x1/self._parent.motor.stepSizeA
        if x2 is not None:
            payload["x2"] = x2/self._parent.motor.stepSizeA
        if z1 is not None:
            payload["z1"] = z1/self._parent.motor.stepSizeZ
        if z2 is not None:
            payload["z2"] = z2/self._parent.motor.stepSizeZ

        nResponses = 1 if isBlocking else 0
        r = self._parent.post_json(
            path, 
            payload, 
            getReturn=isBlocking, 
            timeout=self.timeout if isBlocking else 0, 
            nResponses=nResponses
        )
        return r
