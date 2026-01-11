import numpy as np
import time
import json


gTIMEOUT = 100 # seconds to wait for a response from the ESP32
class Motor(object):

    # indicate if there is any motion happening
    isRunning = False

    def __init__(self, parent=None):
        self._parent = parent

        # do we have a coreXY setup?
        self.isCoreXY = False

        self.nMotors = 4
        self.lastDirection = np.zeros((self.nMotors))
        self.backlash = np.zeros((self.nMotors))
        self.stepSize = np.ones((self.nMotors))
        self.maxStep = np.ones((self.nMotors))*np.inf
        self.minStep = np.ones((self.nMotors))*(-np.inf)
        self.currentDirection = np.zeros((self.nMotors))
        self.currentPosition = np.zeros((self.nMotors))

        self.minPosX = -np.inf
        self.minPosY = -np.inf
        self.minPosZ = -np.inf
        self.minPosA = -np.inf
        self.maxPosX = np.inf
        self.maxPosY = np.inf
        self.maxPosZ = np.inf
        self.maxPosA = np.inf
        self.stepSizeX =  1
        self.stepSizeY =  1
        self.stepSizeZ =  1
        self.stepSizeA =  1

        self.DEFAULT_ACCELERATION = 1000000

        self.motorAxisOrder = [0,1,2,3] # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded

        # register a callback function for the motor status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_motor_status, pattern="steppers")
            # Register callback for stagescan completion signal: {"stagescan":{},"qid":0,"success":1}
            self._parent.serial.register_callback(self._callback_stagescan_complete, pattern="stagescan")
        # announce a function that is called when we receive a position update through the callback
        self._callbackPerKey = {}
        self.nCallbacks = 10
        self._callbackPerKey = self.init_callback_functions(nCallbacks=self.nCallbacks) # only one is used for now
        print(self._callbackPerKey)
        
        # Stage scan completion state
        self._stagescan_complete = False
        self._stagescan_callbacks = []  # List of callbacks to call when stagescan completes
        # move motor to wake them up #FIXME: Should not be necessary!
        #self.move_stepper(steps=(1,1,1,1), speed=(1000,1000,1000,1000), is_absolute=(False,False,False,False))
        #self.move_stepper(steps=(-1,-1,-1,-1), speed=(1000,1000,1000,1000), is_absolute=(False,False,False,False))

    def init_callback_functions(self, nCallbacks=10):
        ''' initialize the callback functions '''
        _callbackPerKey = {}
        self.nCallbacks = nCallbacks
        for i in range(nCallbacks):
            _callbackPerKey[i] = []
        return _callbackPerKey
            
    def _callback_motor_status(self, data):
        ''' cast the json in the form:
        {
        "qid":	0,
        "steppers":	[{
                "stepperid":	1,
                "position":	1000,
                "isDone":	1
            }]
        }
        into the position array of the motors '''
        try:
            nSteppers = len(data["steppers"])
            stepSizes = np.array((self.stepSizeA, self.stepSizeX, self.stepSizeY, self.stepSizeZ))
            for iMotor in range(nSteppers):
                stepperID = data["steppers"][iMotor]["stepperid"]
                # Hardware returns steps, convert to physical units: (steps * stepSize)
                self.currentPosition[stepperID] = data["steppers"][iMotor]["position"] * stepSizes[stepperID]
            if  callable(self._callbackPerKey[0]):
                self._callbackPerKey[0](self.currentPosition) # we call the function with the value
        except Exception as e:
            print("Error in _callback_motor_status: ", e)

    def _callback_stagescan_complete(self, data):
        """
        Callback for stagescan completion signal from firmware.
        
        Expected JSON format:
        {
            "stagescan": {},
            "qid": 0,
            "success": 1
        }
        
        Args:
            data: JSON data dictionary from firmware
        """
        try:
            # Check if this is a completion signal (success key present)
            if "success" in data:
                self._stagescan_complete = True
                success = data.get("success", 0)
                self._parent.logger.debug(f"Stage scan complete signal received: success={success}")
                
                # Call all registered completion callbacks
                for callback in self._stagescan_callbacks:
                    try:
                        callback(data)
                    except Exception as e:
                        self._parent.logger.error(f"Error in stagescan completion callback: {e}")
            else:
                self._parent.logger.debug("Received stagescan data without completion signal.")
        except Exception as e:
            print(f"Error in _callback_stagescan_complete: {e}")

    def register_stagescan_callback(self, callback):
        """
        Register a callback function to be called when stagescan completes.
        
        Args:
            callback: Function to call with completion data
        """
        if callback not in self._stagescan_callbacks:
            self._stagescan_callbacks.append(callback)
    
    def unregister_stagescan_callback(self, callback):
        """
        Unregister a stagescan completion callback.
        
        Args:
            callback: Function to unregister
        """
        if callback in self._stagescan_callbacks:
            self._stagescan_callbacks.remove(callback)

    def reset_stagescan_complete(self):
        """Reset the stagescan completion flag."""
        self._stagescan_complete = False
    
    def is_stagescan_complete(self):
        """Check if stagescan has completed."""
        return self._stagescan_complete

    def wait_for_stagescan_complete(self, timeout=300):
        """
        Wait for stagescan to complete with timeout.
        
        Args:
            timeout: Maximum time to wait in seconds
            
        Returns:
            True if completed, False if timeout
        """
        start_time = time.time()
        while not self._stagescan_complete:
            if time.time() - start_time > timeout:
                return False
            time.sleep(0.1)
        return True

    def register_callback(self, key, callbackfct):
        ''' register a callback function for a specific key '''
        self._callbackPerKey[key] = callbackfct
        
        
    def setTrigger(self, axis="X", pin=1, offset=0, period=1):
        # {"task": "/motor_act", "setTrig": {"steppers": [{"stepperid": 1, "trigPin": 1, "trigOff":0, "trigPer":1}]}}
        if type(axis) is not int:
            axis = self.xyztTo1230(axis)
        path = "/motor_act"
        payload = {
            "task": path,
            "setTrig":{
                "steppers": [
                {
                 "stepperid": axis,
                 "trigPin": pin,
                 "trigOff": offset,
                 "trigPer": period
                 }]
            }}
        r = self._parent.post_json(path, payload)
        return r

    # {"task": "/motor_act", "stagescan": {"nStepsLine": 50, "dStepsLine": 1, "nTriggerLine": 1, "nStepsPixel": 50, "dStepsPixel": 1, "nTriggerPixel": 1, "delayTimeStep": 10, "stopped": 0, "nFrames": 50}}"}}
    def startStageScanning(self, nStepsLine=100, dStepsLine=1, nTriggerLine=1, nStepsPixel=100, dStepsPixel=1, nTriggerPixel=1, delayTimeStep=10, nFrames=5, isBlocking = False):
        path = "/motor_act"
        payload = {
            "task": path,
            "stagescan":{
                "nStepsLine": nStepsLine,
                "dStepsLine": dStepsLine,
                "nTriggerLine": nTriggerLine,
                "nStepsPixel": nStepsPixel,
                "dStepsPixel": dStepsPixel,
                "nTriggerPixel": nTriggerPixel,
                "delayTimeStep": delayTimeStep,
                "stopped": 0,
                "nFrames": nFrames
            }}
        r = self._parent.post_json(path, payload, getReturn=isBlocking)
        return r

    def stopStageScanning(self):
        self.startStageScanning(stopped=1)

    def setIsCoreXY(self, isCoreXY = False):
        self.isCoreXY = isCoreXY

    def setMotorAxisOrder(self, order=[0,1,2,3]):
        self.motorAxisOrder = order

    # { "task": "/motor_act", "focusscan": { "zStart": 0, "zStep": 50, "nZ": 20, "tPre": 80, "tTrig": 20, "tPost": 0, "led": 0, "illumination": [0, 255, 0, 0], "speed": 20000, "acceleration": 1000000, "qid": 42 }}
    def startFocusScanning(self, zStart=0, zStep=50, nZ=20, tPre=80, tTrig=20, tPost=0, led=0, illumination=[0, 255, 0, 0], speed=20000, acceleration=1000000, qid=42):
        path = "/motor_act"
        payload = {
            "task": path,
            "focusscan": {
                "zStart": zStart,
                "zStep": zStep,
                "nZ": nZ,
                "tPre": tPre,
                "tTrig": tTrig,
                "tPost": tPost,
                "led": led,
                "illumination": illumination,
                "speed": speed,
                "acceleration": acceleration,
                "qid": qid
            }
        }
        r = self._parent.post_json(path, payload)
        return r
    
    def stopFocusScanning(self):
        path = "/motor_act"
        payload = {
            "task": path,
            "focusscan": {
                "stopped": 1
            }
        }
        r = self._parent.post_json(path, payload)
        return r
    '''################################################################################################################################################
    HIGH-LEVEL Functions that rely on basic REST-API functions
    ################################################################################################################################################'''
    def setup_motor(self, axis, minPos, maxPos, stepSize, backlash):
        if axis == "X":
            self.minPosX = minPos
            self.maxPosX = maxPos
            self.stepSizeX = stepSize
        elif axis == "Y":
            self.minPosY = minPos
            self.maxPosY = maxPos
            self.stepSizeY = stepSize
        elif axis == "Z":
            self.minPosZ = minPos
            self.maxPosZ = maxPos
            self.stepSizeZ = stepSize
        elif axis == "A":
            self.minPosA = minPos
            self.maxPosA = maxPos
            self.stepSizeA = stepSize
        self.backlash[self.xyztTo1230(axis)] = backlash

    def xyztTo1230(self, axis):
        axis = axis.upper()
        if axis == "X":
            axis = 1
        if axis == "Y":
            axis = 2
        if axis == "Z":
            axis = 3
        if axis == "A":
            axis = 0
        return axis

    def cartesian2corexy(self, x, y):
        # convert cartesian coordinates to coreXY coordinates
        # https://www.corexy.com/theory.html
        x1 = (x+y)/np.sqrt(2)
        y1 = (x-y)/np.sqrt(2)
        return x1, y1

    def move_x(self, steps=0, speed=1000, acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT, is_reduced=False):
        if self.isCoreXY:
            # have to turn two motors to move in X direction
            xTemp, yTemp =  self.cartesian2corexy(steps, 0)
            return self.move_xy(steps=(xTemp, yTemp), speed=(speed,speed), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)
        else:
            return self.move_axis_by_name(axis="X", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)

    def move_y(self, steps=0, speed=1000, acceleration=None,  is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT, is_reduced=False):
        if self.isCoreXY:
            # have to turn two motors to move in Y direction
            xTemp, yTemp =  self.cartesian2corexy(0,steps)
            return self.move_xy(steps=(xTemp, yTemp), speed=(speed,speed), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)
        else:
            return self.move_axis_by_name(axis="Y", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)

    def move_z(self, steps=0, speed=1000, acceleration=None,  is_blocking=False, is_absolute=False, is_dualaxis = False, is_enabled=True, timeout=gTIMEOUT, is_reduced=False):
        if is_dualaxis:
            self.move_az(steps=(steps, steps), speed=(speed,speed), acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=gTIMEOUT, is_reduced=is_reduced)
        else:
            return self.move_axis_by_name(axis="Z", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)

    def move_a(self, steps=0, speed=1000, acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT, is_reduced=False):
        return self.move_axis_by_name(axis="A", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)

    def move_xyz(self, steps=(0,0,0), speed=(1000,1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT, is_reduced=False):
        if len(speed)!= 3:
            speed = (speed,speed,speed)

        # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded
        r = self.move_xyza(steps=(0,steps[0],steps[1],steps[2]), acceleration=(0,acceleration[0],acceleration[1],acceleration[2]), speed=(0,speed[0],speed[1],speed[2]), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)
        return r

    def move_xy(self, steps=(0,0), speed=(1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT, is_reduced=False):
        if self.isCoreXY:
            # have to move only one motor to move in XY direction
           return self.move_xyza(steps=(0,steps[0], steps[1], 0), speed=(0,speed[0],speed[1],0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)
        else:
            if type(speed)==int or len(speed)!= 2:
                speed = (speed,speed)

            if acceleration is None:
                acceleration = 100000
                
            if type(acceleration)==int or len(acceleration)!= 2:
                acceleration = (acceleration,acceleration)

            # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded
            r = self.move_xyza(steps=(0, steps[0],steps[1],0), speed=(0,speed[0],speed[1],0), acceleration=(0,acceleration[0],acceleration[1],0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)
            return r

    def move_az(self, steps=(0,0), speed=(1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT, is_reduced=False):
        if (type(speed)!=list and type(speed)!=tuple) or len(speed)!= 2:
            speed = (speed,speed)

        if (type(acceleration)!=list and type(acceleration)!=tuple) or len(acceleration)!= 2:
            acceleration = (acceleration,acceleration)

        # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded
        r = self.move_xyza(steps=(steps[0],0,0,steps[1]), speed=(speed[0],0,0,speed[1]), acceleration=(acceleration[0],0,0,acceleration[1]), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)
        return r

    def move_xyza(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT, is_reduced=False):
        # everywhere, where relative steps are zero, speed should be zero, too
        if type(speed)==int:
            speed = [speed,speed,speed,speed]
        if type(speed)==tuple:
            speed = list(speed)
        for iMotor in range(len(steps)):
            if steps[iMotor]==0 and not is_absolute:
                speed[iMotor] = 0

        r = self.move_stepper(steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout, is_reduced=is_reduced)
        return r

    def move_axis_by_name(self, axis="X", steps=100, speed=1000, acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT, is_reduced=False):
        axis = self.xyztTo1230(axis)
        _speed=np.zeros(4)
        _speed[axis] = speed
        _steps=np.array((0,0,0,0))
        _steps[axis] = steps
        _acceleration=acceleration
        r = self.move_stepper(_steps, speed=_speed, acceleration=_acceleration, timeout=timeout, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, is_reduced=is_reduced)
        return r

    def move_forever(self, speed=(0,0,0,0), is_stop=False, is_blocking=False):
        if type(speed)==int:
            speed=(speed, speed, speed, speed)
        if len(speed)==3:
            speed = (*speed,0)
        '''
        {"task":"/motor_act",
            "motor":
            {
                "steppers": [
                    { "stepperid": 3, "isforever": 1, "speed": 2000}
                ]
            }
        }
        '''
        # only consider those actions that are necessary
        motorPropList = []
        for iMotor in range(4):
            if abs(speed[iMotor])>=0:
                motorProp = { "stepperid": iMotor,
                             "isforever": int(not is_stop),
                             "speed": speed[iMotor]}
                motorPropList.append(motorProp)

        path = "/motor_act"
        payload = {
            "task":path,
            "motor":
            {
                "steppers": motorPropList
            }
        }

        r = self._parent.post_json(path, payload, timeout=0, getReturn=is_blocking)
        return r

    def stop(self, axis=None):
        axisNumberList = []
        if axis is None:
            for iMotor in range(self.nMotors):
                axisNumberList.append(iMotor)
        else:
            axisNumberList.append(self.xyztTo1230(axis))

        motorPropList = []
        for iMotor in axisNumberList:
            motorProp = { "stepperid": self.motorAxisOrder[iMotor],
                            "isstop": True}
            motorPropList.append(motorProp)

        path = "/motor_act"
        payload = {
            "task":path,
            "motor":
            {
                "steppers": motorPropList
            }
        }
        # ensure that nothing blocks this command!
        if self.isRunning:
            self._parent.serial.breakCurrentCommunication()
        r = self._parent.post_json(path, payload, getReturn=False, timeout=0)
        return r

    def compute_travel_time(self, steps, max_speed, acceleration):
        import math
        """
        Calculate approximate travel time for a stepper motor 
        with trapezoidal velocity profile.

        steps       : total distance in steps
        max_speed   : maximum speed in steps/second
        acceleration: acceleration in steps/second^2
        """
        # Distance to accelerate from 0 to max_speed:
        dist_accel = (max_speed ** 2) / (2.0 * acceleration)

        # If distance is too short to reach max speed -> triangular profile
        if steps < 2 * dist_accel:
            # T = 2 * sqrt( distance / acceleration )
            return 2.0 * math.sqrt(steps / acceleration)
        else:
            # Time to accelerate to max_speed
            t_accel = max_speed / acceleration
            # Distance spent accelerating
            d_accel = dist_accel
            # Distance spent decelerating (same as accelerating)
            d_decel = dist_accel
            # Remaining distance at constant speed
            d_const = steps - d_accel - d_decel
            # Time at constant speed
            t_const = d_const / max_speed
            # Total time
            return t_accel + t_const + t_accel



    def move_stepper(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), is_absolute=(False,False,False,False), timeout=gTIMEOUT, acceleration=(None, None, None, None), is_blocking=True, is_enabled=True, is_reduced=False):
        '''
        This tells the motor to run at a given speed for a specific number of steps; Multiple motors can run simultaneously

        XYZT => 1,2,3,0
        '''
        # determine the axis to operate
        axisToMove = np.where(np.abs(speed)>0)[0]
        if axisToMove.shape[0]==0:
            return "{'return':-1}"
        if type(is_absolute)==bool:
            isAbsoluteArray = np.zeros((4))
            isAbsoluteArray[axisToMove] = is_absolute
        else:
            isAbsoluteArray = is_absolute

        # convert single elements to array
        if type(speed)!=list and type(speed)!=tuple and type(speed)!=np.ndarray:
            speed = np.array((speed,speed,speed,speed))

        # convert single elements to array
        if type(acceleration)!=list and type(acceleration)!=tuple and type(acceleration)!=np.ndarray:
            acceleration = np.array((acceleration,acceleration,acceleration,acceleration))

        # make sure value is an array
        if type(steps)==tuple:
            steps = np.array(steps)

        # Store the target position in physical units BEFORE conversion to hardware steps
        targetPositionPhysical = steps.copy()
        
        # convert from physical units to steps
        steps[0] *= 1/self.stepSizeA
        steps[1] *= 1/self.stepSizeX
        steps[2] *= 1/self.stepSizeY
        steps[3] *= 1/self.stepSizeZ
        
        # detect change in direction and compute distances in HARDWARE STEPS for travel time calculation
        absoluteDistances_steps = np.zeros((4))  # Distance in hardware steps
        stepSizes = np.array((self.stepSizeA, self.stepSizeX, self.stepSizeY, self.stepSizeZ))
        
        for iMotor in range(4):
            # for absolute motion:
            if isAbsoluteArray[iMotor]:
                # Compare current position (physical) with target (physical, already includes offset)
                self.currentDirection[iMotor] = 1 if (self.currentPosition[iMotor]  > targetPositionPhysical[iMotor]) else -1
                # Calculate distance to travel in HARDWARE STEPS:
                # Current position (physical) -> convert to steps, then subtract target (already in steps)
                currentPosition_steps = self.currentPosition[iMotor] / stepSizes[iMotor]
                absoluteDistances_steps[iMotor] = abs(currentPosition_steps - steps[iMotor])
            else:
                self.currentDirection[iMotor] = np.sign(steps[iMotor])
                # For relative motion, steps[iMotor] is already the distance in hardware steps
                absoluteDistances_steps[iMotor] = abs(steps[iMotor])
                
            if self.lastDirection[iMotor] != self.currentDirection[iMotor]:
                # we want to overshoot a bit (backlash is in steps, so apply AFTER conversion to steps)
                steps[iMotor] = steps[iMotor] + self.currentDirection[iMotor]*self.backlash[iMotor]
                # Update distance calculation if backlash was applied
                if not isAbsoluteArray[iMotor]:
                    absoluteDistances_steps[iMotor] = abs(steps[iMotor])
    
        # Convert speed and acceleration from physical units to steps/second
        speed_steps = np.zeros(4)
        acceleration_steps = np.zeros(4)
        for iMotor in range(4):
            if speed[iMotor] != 0:
                # Speed: µm/s -> steps/s => divide by stepSize (µm/step)
                speed_steps[iMotor] = abs(speed[iMotor]) # TODO: This is actually given in steps/s / stepSizes[iMotor]
            if acceleration[iMotor] is not None and acceleration[iMotor] != 0:
                # Acceleration: µm/s² -> steps/s² => divide by stepSize
                acceleration_steps[iMotor] = abs(acceleration[iMotor]) # TODO: This is actually given in steps/s / stepSizes[iMotor]
            else:
                # Default acceleration in steps/s²
                acceleration_steps[iMotor] = 20000  # This should also be converted, but we use a safe default
        
        # Calculate travel time using HARDWARE STEPS and converted speed/acceleration
        # Find the axis that will take the longest (limits overall movement time)
        max_travel_time = 0
        for iMotor in range(4):
            if absoluteDistances_steps[iMotor] > 0 and speed_steps[iMotor] > 0:
                axis_time = self.compute_travel_time(
                    absoluteDistances_steps[iMotor], 
                    speed_steps[iMotor], 
                    acceleration_steps[iMotor]
                )
                max_travel_time = max(max_travel_time, axis_time)
        
        # Set timeout based on calculated travel time (add 2 seconds safety margin)
        if max_travel_time > 0:
            timeout = np.uint8(abs(timeout) > 0) * (max_travel_time + 2)
        else:
            timeout = np.uint8(abs(timeout) > 0) * 3  # Minimum 3 seconds if no movement detected
        # get current position
        #_positions = self.get_position() # x,y,z,t = 1,2,3,0
        #pos_3, pos_0, pos_1, pos_2 = _positions[0],_positions[1],_positions[2],_positions[3]


        '''
        # check if within limits
        if pos_0+steps_0 > self.maxPosX or pos_0+steps_0 < self.minPosX:
            steps_0=0
        if pos_1+steps_1 > self.maxPosY or pos_1+steps_1 < self.minPosY:
            steps_1=0
        if pos_2+steps_2 > self.maxPosZ or pos_2+steps_2 < self.minPosZ:
            steps_2 = 0
        if pos_3+steps_3 > self.maxPosA or pos_3+steps_3 < self.minPosA:
            steps_3 = 0
        '''


        # only consider those actions that are necessary
        motorPropList = []
        for iMotor in range(4):
            if isAbsoluteArray[iMotor] or abs(steps[iMotor])>0:
                # if we are absolute and the last target position is the same as the current one, we don't need to move
                # Compare in physical units: targetPositionPhysical vs currentPosition
                if isAbsoluteArray[iMotor] and abs(targetPositionPhysical[iMotor] - self.currentPosition[iMotor])<1:
                    if self._parent.serial.DEBUG: self._parent.logger.debug(f"Motor {iMotor} is already at target position {targetPositionPhysical[iMotor]}")
                    continue
                motorProp = { "stepperid": int(self.motorAxisOrder[iMotor]),
                             "position": int(steps[iMotor]),
                             "speed": int(speed[iMotor]),
                             "isabs": int(isAbsoluteArray[iMotor]),
                             "isaccel": int(1),
                             "isen": int(is_enabled), 
                             "redu": int(is_reduced)}
                if acceleration[iMotor] is not None:
                    motorProp["accel"] = int(acceleration[iMotor])
                else:
                    motorProp["accel"] = self.DEFAULT_ACCELERATION
                motorPropList.append(motorProp)
        if len(motorPropList)==0:
            return "{'return':-1}"
        path = "/motor_act"
        payload = {
            "task":path,
            "motor":
            {
                "steppers": motorPropList
            }
        }

        # Update currentPosition to track expected position in physical units
        # steps are now in hardware units, so we need to convert back to physical
        stepSizes = np.array((self.stepSizeA, self.stepSizeX, self.stepSizeY, self.stepSizeZ))
        for iMotor in range(self.nMotors):
            if isAbsoluteArray[iMotor]:
                # For absolute: convert hardware steps back to physical units: (steps * stepSize)
                self.currentPosition[iMotor] = steps[iMotor] * stepSizes[iMotor]
            else:
                # For relative: convert step delta to physical delta and add to current position
                self.currentPosition[iMotor] = self.currentPosition[iMotor] + (steps[iMotor] * stepSizes[iMotor])

        # drive motor
        self.isRunning = True
        is_blocking = not self._parent.is_wifi and is_blocking and self._parent.serial.is_connected
        timeout = timeout if is_blocking else 0
        nResponses = len(payload["motor"]["steppers"]) + 1
        # if we get a return, we will receive the latest position feedback from the driver  by means of the axis that moves the longest
        r = self._parent.post_json(path, payload, getReturn=is_blocking, timeout=timeout, nResponses=nResponses)


        # save direction for last iteration
        self.lastDirection = self.currentDirection.copy()

        # Reset busy flag
        self.isRunning = False

        return r

    def isBusy(self, steps, timeout=1):
        path = "/motor_get"
        payload = {
            "task":path,
            "isbusy": 1
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try:
            isbusy = 0
            for iMotor in range(self.nMotors):
                isbusy += r["motor"]["steppers"][iMotor]["isbusy"]
            if isbusy:
                return True
            else :
                return False
        except Exception as e:
            return False



    def set_motor(self, stepperid = 0, position = None, stepPin=None, dirPin=None, enablePin=None, maxPos=None, minPos=None, acceleration=None, isEnable=None, isBlocking=True, timeout=2):
        path = "/motor_set"
        payload = {"task":path,
                    "motor":{
                    "steppers": [
                        {   "stepperid": self.motorAxisOrder[stepperid]}]
                    }}

        if stepPin is not None: payload['motor']["steppers"][0]["step"] = stepPin
        if dirPin is not None: payload['motor']["steppers"][0]["dir"] = dirPin
        if enablePin is not None: payload['motor']["steppers"][0]["enable"] = enablePin
        if maxPos is not None: payload['motor']["steppers"][0]["max_pos"] = maxPos
        if minPos is not None: payload['motor']["steppers"][0]["min_pos"] = minPos
        if position is not None: payload['motor']["steppers"][0]["position"] = position
        if acceleration is not None: payload['motor']["steppers"][0]["acceleration"] = acceleration
        if isEnable is not None: payload['motor']["steppers"][0]["isen"] = isEnable

        # send command
        r = self._parent.post_json(path, payload, timeout=1)

        # wait until job has been done
        time0=time.time()
        if isBlocking:
            while self.isBusy((0,0,0,0)):
                time.sleep(0.1)
                if time.time()-time0>timeout:
                    break

        return r

    def set_motor_currentPosition(self, axis=0, currentPosition=10000):
        if type(axis)==str:
            axis = self.xyztTo1230(axis)

        r = self.set_motor(stepperid = axis, position = currentPosition)
        return r

    def set_motor_acceleration(self, axis=0, acceleration=40000):
        if type(axis)==str:
            axis = self.xyztTo1230(axis)

        path = "/motor_act"
        payload = {
            "task": path,
            "motor":
            {
            "steppers": [
            { "stepperid":axis, "isaccel":1, "accel":acceleration}
            ]
            }
            }
        r = self._parent.post_json(path, payload)
        return r


    def set_motor_enable(self, enable=None, enableauto=None):
        """
        turns on/off enable pin overrides motor settings - god for cooling puproses
        eanbaleauto  turns on/off timer of the accelstepper library
        """
        path = "/motor_act"
        payload = {
            "task": path
        }
        if enable is not None:
            payload["isen"] = enable
        if enableauto is not None:
            payload["isenauto"] = enableauto
        if not enableauto:
            enable = True
        r = self._parent.post_json(path, payload)
        return r

    def get_position(self, axis=None, timeout=1):
        # pulls all current positions from the stepper controller
        path = "/motor_get"
        payload = {
            "task":path,
            "position":True,
        }
        _position = np.array((0.,0.,0.,0.)) # T,X,Y,Z
        _physicalStepSizes = np.array((self.stepSizeA, self.stepSizeX, self.stepSizeY, self.stepSizeZ))

        # this may be an asynchronous call.. #FIXME!
        r = self._parent.post_json(path, payload, getReturn = True, nResponses=1, timeout=timeout)[0]
        # returns {"motor": }
        if "motor" in r :
            for index, istepper in enumerate(r["motor"]["steppers"]):
                if index >3: break # TODO: We would need to handle other values too soon
                _position[istepper["stepperid"]]=istepper["position"]*_physicalStepSizes[self.motorAxisOrder[index]]


        return _position

    def set_position(self, axis=1, position=0, timeout=1):

        '''
        {"task":"/motor_act",  "setpos": {"steppers": [{"stepperid":1, "posval": 0}]}}
        '''
        path = "/motor_act"
        if type(axis) !=int:
            axis = self.xyztTo1230(axis)

        payload = {
            "task": path,
            "setpos":{
                "steppers": [
                {
                 "stepperid": axis,
                 "posval": int(position)
                 }]
            }}
        r = self._parent.post_json(path, payload, timeout=timeout)

        return r

    def set_soft_limits(self, axis=1, min_pos=None, max_pos=None, is_enabled=None, timeout=1):
        '''
        Set soft limits for a motor axis. Soft limits prevent motor movement beyond specified boundaries.
        
        Parameters:
        -----------
        axis : int or str
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z")
        min_pos : int, optional
            Minimum position limit in steps
        max_pos : int, optional  
            Maximum position limit in steps
        is_enabled : bool or int, optional
            Enable (1/True) or disable (0/False) soft limits for this axis
        timeout : int
            Command timeout in seconds
            
        Returns:
        --------
        Response from ESP32
        
        Example:
        --------
        # Set limits for X-axis
        motor.set_soft_limits(axis="X", min_pos=-10000, max_pos=10000, is_enabled=True)
        # Disable limits for Z-axis
        motor.set_soft_limits(axis="Z", is_enabled=False)
        
        Note:
        -----
        Soft limits are automatically ignored during homing operations.
        '''
        if type(axis) != int:
            axis = self.xyztTo1230(axis)
        
        path = "/motor_act"
        payload = {
            "task": path,
            "softlimits": {
                "steppers": [{
                    "stepperid": axis
                }]
            }
        }
        
        if min_pos is not None:
            payload["softlimits"]["steppers"][0]["min"] = int(min_pos)
        if max_pos is not None:
            payload["softlimits"]["steppers"][0]["max"] = int(max_pos)
        if is_enabled is not None:
            payload["softlimits"]["steppers"][0]["isen"] = int(is_enabled)
            
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def get_soft_limits(self, axis=None, timeout=1):
        '''
        Get current soft limits configuration for all axes or a specific axis.
        
        Parameters:
        -----------
        axis : int or str, optional
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z"). If None, returns all axes.
        timeout : int
            Command timeout in seconds
            
        Returns:
        --------
        dict : Soft limits configuration
            Contains min, max, and isen (enabled) for each axis
            
        Example:
        --------
        # Get limits for all axes
        limits = motor.get_soft_limits()
        # Get limits for X-axis only  
        x_limits = motor.get_soft_limits(axis="X")
        '''
        motors = self.get_motors(timeout=timeout)
        
        if motors and "steppers" in motors:
            if axis is not None:
                if type(axis) != int:
                    axis = self.xyztTo1230(axis)
                # Find the specific axis
                for stepper in motors["steppers"]:
                    if stepper.get("stepperid") == axis:
                        return {
                            "axis": axis,
                            "min": stepper.get("min", 0),
                            "max": stepper.get("max", 0),
                            "enabled": stepper.get("isen", 0)
                        }
            else:
                # Return all axes
                result = []
                for stepper in motors["steppers"]:
                    result.append({
                        "axis": stepper.get("stepperid"),
                        "min": stepper.get("min", 0),
                        "max": stepper.get("max", 0),
                        "enabled": stepper.get("isen", 0)
                    })
                return result
        return None

    def enable_soft_limits(self, axis, timeout=1):
        '''
        Enable soft limits for a specific axis.
        
        Parameters:
        -----------
        axis : int or str
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z")
        timeout : int
            Command timeout in seconds
        '''
        return self.set_soft_limits(axis=axis, is_enabled=True, timeout=timeout)

    def disable_soft_limits(self, axis, timeout=1):
        '''
        Disable soft limits for a specific axis.
        
        Parameters:
        -----------
        axis : int or str
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z")
        timeout : int
            Command timeout in seconds
        '''
        return self.set_soft_limits(axis=axis, is_enabled=False, timeout=timeout)

    def set_joystick_direction(self, axis, inverted=False, timeout=1):
        '''
        Set joystick direction inversion for a specific motor axis.
        When inverted is True, joystick movements for this axis will be reversed.
        
        Parameters:
        -----------
        axis : int or str
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z")
        inverted : bool or int
            True/1 to invert joystick direction, False/0 for normal direction
        timeout : int
            Command timeout in seconds
            
        Returns:
        --------
        Response from ESP32
        
        Example:
        --------
        # Invert joystick direction for X-axis
        motor.set_joystick_direction(axis="X", inverted=True)
        # Set normal direction for Y-axis  
        motor.set_joystick_direction(axis="Y", inverted=False)
        '''
        if type(axis) != int:
            axis = self.xyztTo1230(axis)
        
        path = "/motor_act"
        payload = {
            "task": path,
            "joystickdir": {
                "steppers": [{
                    "stepperid": axis,
                    "inverted": int(inverted)
                }]
            }
        }
        
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def get_joystick_direction(self, axis=None, timeout=1):
        '''
        Get joystick direction configuration for axes.
        
        Parameters:
        -----------
        axis : int or str, optional
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z"). If None, returns all axes.
        timeout : int
            Command timeout in seconds
            
        Returns:
        --------
        dict or bool : Joystick direction configuration
            Returns inverted status for the specified axis or all axes
            
        Example:
        --------
        # Get direction for X-axis
        x_inverted = motor.get_joystick_direction(axis="X")
        # Get direction for all axes
        all_directions = motor.get_joystick_direction()
        '''
        motors = self.get_motors(timeout=timeout)
        
        if motors and "steppers" in motors:
            if axis is not None:
                if type(axis) != int:
                    axis = self.xyztTo1230(axis)
                # Find the specific axis
                for stepper in motors["steppers"]:
                    if stepper.get("stepperid") == axis:
                        return stepper.get("joystickDirectionInverted", False)
            else:
                # Return all axes
                result = []
                for stepper in motors["steppers"]:
                    result.append({
                        "axis": stepper.get("stepperid"),
                        "inverted": stepper.get("joystickDirectionInverted", False)
                    })
                return result
        return None

    def get_motor(self, axis=1, timeout=1):
        path = "/motor_get"
        payload = {
            "task":path,
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try: return r["steppers"][self.motorAxisOrder[axis]]
        except: return False

    def get_motors(self, timeout=1):
        path = "/motor_get"

        r = self._parent.get_json(path, timeout=timeout)
        try: return r["steppers"]
        except: return False


    def set_direction(self, axis=1, sign=1, timeout=1):
        return False
    
    def stop_stage_scanning(self):
        # {"task":"/motor_act", "stagescan":{ "stopped":1 }}
        path = "/motor_act"
        payload = {
            "task": path,
            "stagescan": {
                "stopped": 1
            }
        }
        r = self._parent.post_json(path, payload)
        return r
    
    

    def start_stage_scanning(self, xstart=0, xstep=1000, nx=20, ystart=0, ystep=1000, ny=10, zstart=0, zstep=1000, nz=10, tsettle=5, tExposure=50, illumination=(0,0,0,0), led=0, speed=20000, acceleration=None):
		#	{"task": "/motor_act", "stagescan": {"xStart": 0, "yStart": 0, "zStart": 0, "xStep": 500, "yStep": 500, "zStep": 500, "nX": 10, "nY": 10, "nZ": 10, "tPre": 50, "tPost": 50, "illumination": [0, 1, 0, 0], "led": 255}}
        if acceleration is None:
            acceleration = self.DEFAULT_ACCELERATION
        path = "/motor_act"
        payload = {
            "task": path,
            "stagescan": {
                "xStart": xstart / self.stepSizeX,
                "xStep": xstep / self.stepSizeX,
                "nX": nx,
                "yStart": ystart / self.stepSizeY,
                "yStep": ystep / self.stepSizeY,
                "nY": ny,
                "tPre": tsettle,
                "tPost": tExposure,
                "illumination": illumination,
                "led": led,
                "accel": self.DEFAULT_ACCELERATION,  # default acceleration
                "speed": speed,  # default speed
                "zStart": zstart / self.stepSizeZ,
                "zStep": zstep / self.stepSizeZ,
                "nZ": nz,
            }
        }
        r = self._parent.post_json(path, payload)
        return r
    
    def start_stage_scanning_by_coordinates(self, coordinates, tPre=50, tPost=50, led=100, illumination=[50, 75, 100, 125], stopped=0): 
        '''
        Example: {"task": "/motor_act", "stagescan": {"coordinates": [{"x": 100, "y": 200, "z": 0}, {"x": 300, "y": 400, "z": 0}, {"x": 500, "y": 600, "z": 0}], "tPre": 50, "tPost": 50, "led": 100, "illumination": [50, 75, 100, 125], "stopped": 0}}
        
        coordinates: list of dictionaries with x, y and z coordinates
        tPre: time before exposure in ms
        tPost: exposure time - time after action
        led: led value for illumination
        illumination: list of values for each channel of the illumination
        stopped: 0 for start, 1 for stop
        '''
        path = "/motor_act"
        payload = {
            "task": path,
            "stagescan": {
                "coordinates": coordinates,
                "tPre": tPre,
                "tPost": tPost,
                "led": led,
                "illumination": illumination,
                "stopped": stopped
            }
        }
        r = self._parent.post_json(path, payload)
        return r
            
    def set_tmc_parameters(self, axis=0, msteps=None, rms_current=None, stall_value=None, sgthrs=None, semin=None, semax=None, blank_time=None, toff=None, timeout=1):
        ''' set the TMC parameters for a specific axis 
        msteps: microsteps
        rms_current: current in mA
        sgthrs: stallguard threshold
        semin: minimum current
        semax: maximum current
        blank_time: blank time
        toff: off time
        #   {"task":"/tmc_act", "msteps":32, "rms_current":600, "sgthrs":100, "semin":5, "semax":2, "blank_time":24, "toff":4, "axis":1}
        '''
        if type(axis)==str:
            axis = self.xyztTo1230(axis)
        path = "/tmc_act"
        payload = {}
        if axis is not None:
            payload["axis"] = axis
        if msteps is not None:
            payload["msteps"] = msteps
        if rms_current is not None:
            payload["rms_current"] = rms_current
        if sgthrs is not None:
            payload["sgthrs"] = sgthrs
        if semin is not None:
            payload["semin"] = semin
        if semax is not None:
            payload["semax"] = semax
        if blank_time is not None:
            payload["blank_time"] = blank_time
        if toff is not None:
            payload["toff"] = toff

        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def set_hard_limits(self, axis=1, enabled=True, polarity=0, timeout=1):
        '''
        Configure hard limits (emergency stop) for a motor axis.
        Hard limits immediately stop the motor when an endstop is triggered during normal operation.
        This is different from homing - the motor will stop and position will be set to 999999 (error state).
        User must perform homing to clear the error state.
        
        Parameters:
        -----------
        axis : int or str
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z")
        enabled : bool or int
            True/1 to enable hard limit protection (default), False/0 to disable
        polarity : int
            Endstop polarity configuration:
            0 = Normally Open (NO) - endstop signal is LOW when not pressed, HIGH when pressed
            1 = Normally Closed (NC) - endstop signal is HIGH when not pressed, LOW when pressed
        timeout : int
            Command timeout in seconds
            
        Returns:
        --------
        Response from ESP32
        
        Example:
        --------
        # Enable hard limit protection for X-axis with normally open endstop
        motor.set_hard_limits(axis="X", enabled=True, polarity=0)
        # Disable hard limit protection for Z-axis
        motor.set_hard_limits(axis="Z", enabled=False)
        # Configure hard limit for normally closed endstop
        motor.set_hard_limits(axis="Y", enabled=True, polarity=1)
        
        Note:
        -----
        - Hard limits are enabled by default on startup
        - When triggered, motor position is set to 999999 to indicate error state
        - Homing automatically clears the hard limit triggered flag
        - Hard limits are ignored during homing operations
        '''
        if type(axis) != int:
            axis = self.xyztTo1230(axis)
        
        path = "/motor_act"
        payload = {
            "task": path,
            "hardlimits": {
                "steppers": [{
                    "stepperid": axis,
                    "enabled": int(enabled),
                    "polarity": int(polarity)
                }]
            }
        }
        
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def clear_hard_limit(self, axis=1, timeout=1):
        '''
        Clear the hard limit triggered flag for a motor axis.
        This is typically done automatically during homing, but can be called manually if needed.
        
        Parameters:
        -----------
        axis : int or str
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z")
        timeout : int
            Command timeout in seconds
            
        Returns:
        --------
        Response from ESP32
        
        Example:
        --------
        motor.clear_hard_limit(axis="X")
        
        Note:
        -----
        After clearing the flag, you should perform homing to establish a known position.
        '''
        if type(axis) != int:
            axis = self.xyztTo1230(axis)
        
        path = "/motor_act"
        payload = {
            "task": path,
            "hardlimits": {
                "steppers": [{
                    "stepperid": axis,
                    "clear": 1
                }]
            }
        }
        
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def get_hard_limits(self, axis=None, timeout=1):
        '''
        Get hard limit configuration and status for axes.
        
        Parameters:
        -----------
        axis : int or str, optional
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z"). If None, returns all axes.
        timeout : int
            Command timeout in seconds
            
        Returns:
        --------
        dict or list : Hard limit configuration
            Contains enabled, polarity, and triggered status for the specified axis or all axes
            
        Example:
        --------
        # Get hard limit status for X-axis
        x_limits = motor.get_hard_limits(axis="X")
        # Returns: {"axis": 1, "enabled": True, "polarity": 0, "triggered": False}
        
        # Get hard limit status for all axes
        all_limits = motor.get_hard_limits()
        
        # Check if any axis has triggered a hard limit
        for limit in motor.get_hard_limits():
            if limit["triggered"]:
                print(f"Hard limit triggered on axis {limit['axis']}! Homing required.")
        '''
        motors = self.get_motors(timeout=timeout)
        
        if motors and "steppers" in motors:
            if axis is not None:
                if type(axis) != int:
                    axis = self.xyztTo1230(axis)
                # Find the specific axis
                for stepper in motors["steppers"]:
                    if stepper.get("stepperid") == axis:
                        return {
                            "axis": axis,
                            "enabled": bool(stepper.get("hardLimitEnabled", True)),
                            "polarity": stepper.get("hardLimitPolarity", 0),
                            "triggered": bool(stepper.get("hardLimitTriggered", False))
                        }
            else:
                # Return all axes
                result = []
                for stepper in motors["steppers"]:
                    result.append({
                        "axis": stepper.get("stepperid"),
                        "enabled": bool(stepper.get("hardLimitEnabled", True)),
                        "polarity": stepper.get("hardLimitPolarity", 0),
                        "triggered": bool(stepper.get("hardLimitTriggered", False))
                    })
                return result
        return None

    def is_hard_limit_triggered(self, axis=None, timeout=1):
        '''
        Check if a hard limit has been triggered for one or more axes.
        
        Parameters:
        -----------
        axis : int or str, optional
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z"). If None, checks all axes.
        timeout : int
            Command timeout in seconds
            
        Returns:
        --------
        bool : True if hard limit is triggered for the specified axis
        dict : If axis is None, returns {axis_id: triggered_status} for all axes
            
        Example:
        --------
        # Check if X-axis hard limit is triggered
        if motor.is_hard_limit_triggered(axis="X"):
            print("X-axis hard limit triggered! Performing homing...")
            motor.home_x()
            
        # Check all axes
        triggered = motor.is_hard_limit_triggered()
        for ax, is_triggered in triggered.items():
            if is_triggered:
                print(f"Axis {ax} needs homing!")
        '''
        limits = self.get_hard_limits(axis=axis, timeout=timeout)
        
        if limits is None:
            return None
            
        if axis is not None:
            return limits.get("triggered", False)
        else:
            result = {}
            for limit in limits:
                result[limit["axis"]] = limit["triggered"]
            return result

    def enable_hard_limits(self, axis=None, timeout=1):
        '''
        Enable hard limit protection for one or all axes.
        
        Parameters:
        -----------
        axis : int or str, optional
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z"). If None, enables for X, Y, Z.
        timeout : int
            Command timeout in seconds
        '''
        if axis is not None:
            return self.set_hard_limits(axis=axis, enabled=True, timeout=timeout)
        else:
            # Enable for X, Y, Z axes
            for ax in ["X", "Y", "Z"]:
                self.set_hard_limits(axis=ax, enabled=True, timeout=timeout)

    def disable_hard_limits(self, axis=None, timeout=1):
        '''
        Disable hard limit protection for one or all axes.
        Use with caution - the motor will not stop if an endstop is hit during normal operation.
        
        Parameters:
        -----------
        axis : int or str, optional
            Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z"). If None, disables for X, Y, Z.
        timeout : int
            Command timeout in seconds
        '''
        if axis is not None:
            return self.set_hard_limits(axis=axis, enabled=False, timeout=timeout)
        else:
            # Disable for X, Y, Z axes
            for ax in ["X", "Y", "Z"]:
                self.set_hard_limits(axis=ax, enabled=False, timeout=timeout)
