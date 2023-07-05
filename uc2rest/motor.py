import numpy as np
import time
import json


gTIMEOUT = 100 # seconds to wait for a response from the ESP32
class Motor(object):

    # indicate if there is any motion happening
    isRunning = False

    # a dictionary that stores all motor parameters for each dxis
    settingsdict = {"motor": {"steppers":
        [{"stepperid":0,"dir":0,"step":0,"enable":0,"dir_inverted":False,"step_inverted":False,"enable_inverted":False,"speed":0,"speedmax":200000,"max_pos":100000,"min_pos":-100000},
         {"stepperid":1,"dir":0,"step":0,"enable":0,"dir_inverted":False,"step_inverted":False,"enable_inverted":False,"speed":0,"speedmax":200000,"max_pos":100000,"min_pos":-100000},
         {"stepperid":2,"dir":0,"step":0,"enable":0,"dir_inverted":False,"step_inverted":False,"enable_inverted":False,"speed":0,"speedmax":200000,"max_pos":100000,"min_pos":-100000},
         {"stepperid":3,"dir":0,"step":0,"enable":0,"dir_inverted":False,"step_inverted":False,"enable_inverted":False,"speed":0,"speedmax":200000,"max_pos":100000,"min_pos":-100000}]
        }}



    def __init__(self, parent=None):
        self._parent = parent
        
        # do we have a coreXY setup?
        self.isCoreXY = False

        self.nMotors = 4
        self.steps_last = np.zeros((self.nMotors))
        self.backlash = np.zeros((self.nMotors))
        self.stepSize = np.ones((self.nMotors))
        self.maxStep = np.ones((self.nMotors))*np.inf
        self.minStep = np.ones((self.nMotors))*(-np.inf)

        self.minPosX = -np.inf
        self.minPosY = -np.inf
        self.minPosZ = -np.inf
        self.minPosT = -np.inf
        self.maxPosX = np.inf
        self.maxPosY = np.inf
        self.maxPosZ = np.inf
        self.maxPosT = np.inf
        self.stepSizeX =  1
        self.stepSizeY =  1
        self.stepSizeZ =  1
        self.stepSizeT =  1

        self.motorAxisOrder = [0,1,2,3] # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded

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
            self.backlashX = backlash
        elif axis == "Y":
            self.minPosY = minPos
            self.maxPosY = maxPos
            self.stepSizeY = stepSize
            self.backlashY = backlash
        elif axis == "Z":
            self.minPosZ = minPos
            self.maxPosZ = maxPos
            self.stepSizeZ = stepSize
            self.backlashZ = backlash
        elif axis == "T":
            self.minPosT = minPos
            self.maxPosT = maxPos
            self.stepSizeT = stepSize
            self.backlashT = backlash

    def xyztTo1230(self, axis):
        axis = axis.upper()
        if axis == "X":
            axis = 1
        if axis == "Y":
            axis = 2
        if axis == "Z":
            axis = 3
        if axis == "T" or axis == "A":
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

    def move_z(self, steps=0, speed=1000, acceleration=None,  is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        return self.move_axis_by_name(axis="Z", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_t(self, steps=0, speed=1000, acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        return self.move_axis_by_name(axis="T", steps=steps, speed=speed, acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_xyz(self, steps=(0,0,0), speed=(1000,1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if len(speed)!= 3:
            speed = (speed,speed,speed)

        # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded
        r = self.move_xyzt(steps=(0,steps[0],steps[1],steps[2]), acceleration=acceleration, speed=(0,speed[0],speed[1],speed[2]), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        return r

    def move_xy(self, steps=(0,0), speed=(1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if self.isCoreXY:
            # have to move only one motor to move in XY direction
           return self.move_xyzt(steps=(0,steps[0], steps[1], 0), speed=(0,speed[0],speed[1],0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)           
        else:
            if len(speed)!= 2:
                speed = (speed,speed)

            # motor axis is 1,2,3,0 => X,Y,Z,T # FIXME: Hardcoded
            r = self.move_xyzt(steps=(0, steps[0],steps[1],0), speed=(0,speed[0],speed[1],0), acceleration=acceleration, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
            return r

    def move_xyzt(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if type(speed)==int:
            speed = (speed,speed,speed,speed)
        if type(steps)==int:
            steps = (steps,steps,steps,steps)

        r = self.move_stepper(steps=steps, speed=speed, acceleration=acceleration, backlash=(self.backlash[0],self.backlash[1],self.backlash[2],self.backlash[3]), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        return r

    def move_axis_by_name(self, axis="X", steps=100, speed=1000, acceleration=None, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        axis = self.xyztTo1230(axis)
        _speed=np.zeros(4)
        _speed[axis] = speed
        _steps=np.array((0,0,0,0))
        _steps[axis] = steps
        _backlash=np.zeros(4)
        _backlash[axis] = self.backlash[axis]
        _acceleration=acceleration
        r = self.move_stepper(_steps, speed=_speed, acceleration=_acceleration, timeout=timeout, backlash=_backlash, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled)
        return r

    def move_forever(self, speed=(0,0,0,0), is_stop=False):
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

        r = self._parent.post_json(path, payload, timeout=0)
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


    def move_stepper(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), is_absolute=(False,False,False,False), timeout=gTIMEOUT, backlash=(0,0,0,0), acceleration=(None, None, None, None), is_blocking=True, is_enabled=True):
        '''
        This tells the motor to run at a given speed for a specific number of steps; Multiple motors can run simultaneously

        XYZT => 1,2,3,0
        '''

        # determine the axis to operate
        axisToMove = np.where(np.abs(speed)>0)

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

        # detect change in directiongit config --global user.name "Your Name"
        for iMotor in range(4):
            if np.sign(self.steps_last[iMotor]) != np.sign(steps[iMotor]):
                # we want to overshoot a bit
                steps[iMotor] = steps[iMotor] + (np.sign(steps[iMotor])*backlash[iMotor])

        # get current position
        #_positions = self.get_position() # x,y,z,t = 1,2,3,0
        #pos_3, pos_0, pos_1, pos_2 = _positions[0],_positions[1],_positions[2],_positions[3]

        # convert to physical units
        steps[0] *= 1/self.stepSizeT
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
        if pos_3+steps_3 > self.maxPosT or pos_3+steps_3 < self.minPosT:
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
                             "isaccel":0, 
                             "isen": is_enabled}
                if acceleration[iMotor] is not None:
                    motorProp["accel"] = int(acceleration[iMotor])
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
            self.steps_last[iMotor] = steps[iMotor]

        # drive motor
        self.isRunning = True
        r = self._parent.post_json(path, payload, getReturn=is_blocking, timeout=timeout)

        # wait until job has been done        
        time0=time.time()
        if np.sum(isAbsoluteArray):
            steppersRunning = isAbsoluteArray
        else:
            steppersRunning = np.abs(np.array(steps))>0
        if not self._parent.is_wifi and is_blocking and self._parent.serial.is_connected:
            while True:
                time.sleep(0.05) # don'T overwhelm the CPU
                # see if already done
                try:
                    rMessage = self._parent.serial.serialdevice.readline().decode() # TODO: Make sure it's compatible with all motors running at the same time
                except Exception as e:
                    self._parent.logger.error(e)
                    rMessage = ""
                # check if message contains a motor that is done already
                if rMessage.find('isDone') >-1:
                    ''' TODO: This only checks for one motor!'''
                    try:
                        rMessage = rMessage.split("\r")[0].replace("'", '"')
                        mMessage = json.loads(rMessage)
                        for iElement in mMessage['steppers']:
                            if iElement['isDone']:
                                mNumber = self.motorAxisOrder[iElement['stepperid']]
                                steppersRunning[mNumber] = False
                    except:
                        pass
                    
                if np.sum(steppersRunning)==0:
                    break

                if time.time()-time0>timeout:
                    break


        # reset busy flag
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


    def set_motor_enable(self, is_enable=1):
        self.set_motor_enable(enable=is_enable)

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
        r = self._parent.post_json(path, payload)
        return r

    def get_position(self, axis=None, timeout=1):
        # pulls all current positions from the stepper controller
        path = "/motor_get"
        payload = {
            "task":path,
            "position":True,
        }
        _position = np.array((0,0,0,0)) # T,X,Y,Z
        _physicalStepSizes = np.array((self.stepSizeT, self.stepSizeX, self.stepSizeY, self.stepSizeZ))

        # this may be an asynchronous call.. #FIXME!
        for i in range(3):
            if not self._parent.is_sending(): 
                r = self._parent.post_json(path, payload, timeout=timeout)
                if "motor" in r:
                    for index, istepper in enumerate(r["motor"]["steppers"]):
                        _position[istepper["stepperid"]]=istepper["position"]*_physicalStepSizes[self.motorAxisOrder[index]]
                    break

        return _position

    def set_position(self, axis=1, position=0, timeout=1):

        '''
        {"task":"/home_act",  "setpos": {"steppers": [{"stepperid":1, "posval": 0}]}}
        '''
        path = "/home_act"
        if axis=="X": axis=1
        if axis=="Y": axis=2
        if axis=="Z": axis=3

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
