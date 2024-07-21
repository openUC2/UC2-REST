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
        self._position = np.zeros((self.nMotors)) # position from the last motor status update

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

        self.DEFAULT_ACCELERATION = 10000

        self.motorAxisOrder = [0,1,2,3] # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded

        # register a callback function for the motor status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_motor_status, pattern="steppers")
        # announce a function that is called when we receive a position update through the callback
        self._callbackPerKey = {}
        self.nCallbacks = 10
        self._callbackPerKey = self.init_callback_functions(nCallbacks=self.nCallbacks) # only one is used for now
        print(self._callbackPerKey)
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
            for iMotor in range(nSteppers):
                stepperID = data["steppers"][iMotor]["stepperid"]
                # smart to re-update this variable? Will be updated by motor-sender too
                self._position[stepperID] = data["steppers"][iMotor]["position"]
            if  callable(self._callbackPerKey[0]):
                self._callbackPerKey[0](self._position) # we call the function with the value
        except Exception as e:
            print("Error in _callback_motor_status: ", e)

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

    def move_x(self, steps=0, speed=1000, acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if self.isCoreXY:
            # have to turn two motors to move in X direction
            xTemp, yTemp =  self.cartesian2corexy(steps, 0)
            return self.move_xy(steps=(xTemp, yTemp), speed=(speed,speed), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        else:
            return self.move_axis_by_name(axis="X", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_y(self, steps=0, speed=1000, acceleration=None,  is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if self.isCoreXY:
            # have to turn two motors to move in Y direction
            xTemp, yTemp =  self.cartesian2corexy(0,steps)
            return self.move_xy(steps=(xTemp, yTemp), speed=(speed,speed), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        else:
            return self.move_axis_by_name(axis="Y", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_z(self, steps=0, speed=1000, acceleration=None,  is_blocking=False, is_absolute=False, is_dualaxis = False, is_enabled=True, timeout=gTIMEOUT):
        if is_dualaxis:
            self.move_az(steps=(steps, steps), speed=(speed,speed), acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=gTIMEOUT)
        else:
            return self.move_axis_by_name(axis="Z", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_a(self, steps=0, speed=1000, acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        return self.move_axis_by_name(axis="A", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_xyz(self, steps=(0,0,0), speed=(1000,1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if len(speed)!= 3:
            speed = (speed,speed,speed)

        # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded
        r = self.move_xyza(steps=(0,steps[0],steps[1],steps[2]), acceleration=(0,acceleration[0],acceleration[1],acceleration[2]), speed=(0,speed[0],speed[1],speed[2]), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        return r

    def move_xy(self, steps=(0,0), speed=(1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if self.isCoreXY:
            # have to move only one motor to move in XY direction
           return self.move_xyza(steps=(0,steps[0], steps[1], 0), speed=(0,speed[0],speed[1],0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        else:
            if type(speed)==int or len(speed)!= 2:
                speed = (speed,speed)

            if len(acceleration)!= 2:
                acceleration = (acceleration,acceleration)

            # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded
            r = self.move_xyza(steps=(0, steps[0],steps[1],0), speed=(0,speed[0],speed[1],0), acceleration=(0,acceleration[0],acceleration[1],0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
            return r

    def move_az(self, steps=(0,0), speed=(1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if (type(speed)!=list and type(speed)!=tuple) or len(speed)!= 2:
            speed = (speed,speed)

        if (type(acceleration)!=list and type(acceleration)!=tuple) or len(acceleration)!= 2:
            acceleration = (acceleration,acceleration)

        # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded
        r = self.move_xyza(steps=(steps[0],0,0,steps[1]), speed=(speed[0],0,0,speed[1]), acceleration=(acceleration[0],0,0,acceleration[1]), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        return r

    def move_xyza(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if type(speed)==int:
            speed = (speed,speed,speed,speed)
        if type(steps)==int:
            steps = (steps,steps,steps,steps)

        r = self.move_stepper(steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        return r

    def move_axis_by_name(self, axis="X", steps=100, speed=1000, acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        axis = self.xyztTo1230(axis)
        _speed=np.zeros(4)
        _speed[axis] = speed
        _steps=np.array((0,0,0,0))
        _steps[axis] = steps
        _acceleration=acceleration
        r = self.move_stepper(_steps, speed=_speed, acceleration=_acceleration, timeout=timeout, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled)
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
            if abs(speed[iMotor])>0:
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


    def move_stepper(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), is_absolute=(False,False,False,False), timeout=gTIMEOUT, acceleration=(None, None, None, None), is_blocking=True, is_enabled=True):
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

        # detect change in direction
        for iMotor in range(4):
            # for absolute motion:
            if isAbsoluteArray[iMotor]:
                self.currentDirection[iMotor] = 1 if (self.currentPosition[iMotor]  > steps[iMotor]) else -1
            else:
                self.currentDirection[iMotor] = np.sign(steps[iMotor])
            if self.lastDirection[iMotor] != self.currentDirection[iMotor]:
                # we want to overshoot a bit
                steps[iMotor] = steps[iMotor] +  self.currentDirection[iMotor]*self.backlash[iMotor]

        # get current position
        #_positions = self.get_position() # x,y,z,t = 1,2,3,0
        #pos_3, pos_0, pos_1, pos_2 = _positions[0],_positions[1],_positions[2],_positions[3]

        # convert to physical units
        steps[0] *= 1/self.stepSizeA
        steps[1] *= 1/self.stepSizeX
        steps[2] *= 1/self.stepSizeY
        steps[3] *= 1/self.stepSizeZ
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
                motorProp = { "stepperid": self.motorAxisOrder[iMotor],
                             "position": int(steps[iMotor]),
                             "speed": int(speed[iMotor]),
                             "isabs": isAbsoluteArray[iMotor],
                             "isaccel":1,
                             "isen": is_enabled}
                if acceleration[iMotor] is not None:
                    motorProp["accel"] = int(acceleration[iMotor])
                else:
                    motorProp["accel"] = self.DEFAULT_ACCELERATION
                motorPropList.append(motorProp)

        path = "/motor_act"
        payload = {
            "task":path,
            "motor":
            {
                "steppers": motorPropList
            }
        }

        # safe steps to track direction for backlash compensatio
        for iMotor in range(self.nMotors):
            if isAbsoluteArray[iMotor]:
                self.currentPosition[iMotor] = steps[iMotor]
            else:
                self.currentPosition[iMotor] = self.currentPosition[iMotor] + steps[iMotor]

        # drive motor
        self.isRunning = True
        is_blocking = not self._parent.is_wifi and is_blocking and self._parent.serial.is_connected
        timeout = timeout if is_blocking else 0
        if type(axisToMove) == list:
            nResponses = len(axisToMove)+1 # we get the command received flag + a return for every axis
        elif type(axisToMove) == tuple:
            nResponses = axisToMove[0].shape[0]+1
        elif type(axisToMove) == np.ndarray:
            nResponses = axisToMove.shape[0] +1
        else:
            nResponses = 2
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
        r = self._parent.post_json(path, payload, getReturn = True, nResponses=1, timeout=timeout)
        if "motor" in r:
            for index, istepper in enumerate(r["motor"]["steppers"]):
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
