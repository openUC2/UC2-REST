import numpy as np
import time
import json


gTIMEOUT = 10 # seconds to wait for a response from the ESP32
class Motor(object):

    microsteppingfactor_filter=16 # run more smoothly
    filter_pos_1 = 1000*microsteppingfactor_filter # GFP
    filter_pos_2 = 0*microsteppingfactor_filter # AF647/SIR
    filter_pos_3 = 500*microsteppingfactor_filter
    filter_pos_LED = filter_pos_1 # GFP / Brightfield
    filter_pos_init = -1250*microsteppingfactor_filter
    filter_speed = microsteppingfactor_filter * 500
    filter_position_now = 0

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

    def home_x(self):
        r = self.home(axis="X")
        return r

    def home_y(self):
        r = self.home(axis="Y")
        return r

    def home_z(self):
        r = self.home(axis="Z")
        return r

    def home_xyz(self):
        r = self.home(axis="XYZ")
        return r

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

    def home(self, axis="X", timeout=gTIMEOUT, speed = 2000, direction = 1, endposrelease=500):
        # convert X,Y,Z to 0,1,2
        if type(axis) == str:
            axis = self.xyztTo1230(axis)
        if direction not in [-1,1]:
            direction = 1

        path = "/motor_act",

        payload = {
            "task": path,
            "home":{
                "steppers": [
                {
                 "stepperid": axis,
                 "timeout":timeout,
                 "speed":speed,
                 "direction":direction,
                 "endposrelease":endposrelease
                 }]
            }}

        r = self._parent.post_json(path, payload)
        return r


    def move_x(self, steps=100, speed=1000, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        return self.move_axis_by_name(axis="X", steps=steps, speed=speed, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_y(self, steps=100, speed=1000, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        return self.move_axis_by_name(axis="Y", steps=steps, speed=speed, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_z(self, steps=100, speed=1000, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        return self.move_axis_by_name(axis="Z", steps=steps, speed=speed, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_t(self, steps=100, speed=1000, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        return self.move_axis_by_name(axis="T", steps=steps, speed=speed, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)

    def move_xyz(self, steps=(0,0,0), speed=(1000,1000,1000), is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if len(speed)!= 3:
            speed = (speed,speed,speed)

        r = self.move_xyzt(steps=(steps[0],steps[1],steps[2],0), speed=(speed[0],speed[1],speed[2],0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        return r

    def move_xy(self, steps=(0,0), speed=(1000,1000), is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if len(speed)!= 2:
            speed = (speed,speed)

        r = self.move_xyzt(steps=(steps[0],steps[1],0,0), speed=(speed[0],speed[1],0,0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        return r

    def move_xyzt(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        if type(speed)==int:
            speed = (speed,speed,speed,speed)
        if type(steps)==int:
            steps = (steps,steps,steps,steps)

        r = self.move_stepper(steps=steps, speed=speed, backlash=(self.backlash[0],self.backlash[1],self.backlash[2],self.backlash[3]), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled, timeout=timeout)
        return r

    def move_axis_by_name(self, axis="X", steps=100, speed=1000, is_blocking=False, is_absolute=False, is_enabled=True, timeout=gTIMEOUT):
        axis = self.xyztTo1230(axis)
        _speed=np.zeros(4)
        _speed[axis] = speed
        _steps=np.array((0,0,0,0))
        _steps[axis] = steps
        _backlash=np.zeros(4)
        _backlash[axis] = self.backlash[axis]
        r = self.move_stepper(_steps, speed=_speed, timeout=timeout, backlash=_backlash, is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled)
        return r

    def init_filter(self, nSteps, speed=250, filter_axis=-1, is_blocking = True, is_enabled=False):
        self.move_filter(steps=nSteps, speed=speed, filter_axis=filter_axis, is_blocking=is_blocking, is_enabled = is_enabled)
        self.is_filter_init = True
        self.filter_position_now = 0

    def switch_filter(self, filter_pos=0, filter_axis=-1, timeout=20, is_filter_init=None, speed=None, is_enabled=False, is_blocking=True):

        # switch off all lasers first!
        self.set_laser(1, 0)
        self.set_laser(2, 0)
        self.set_laser(3, 0)

        if speed is None:
            speed = self.filter_speed

        if is_filter_init is not None:
            self.is_filter_init = is_filter_init
        if not self.is_filter_init:
            self.init_filter(nSteps=self.filter_pos_init, speed=speed, filter_axis=filter_axis, is_blocking = True)

        # measured in steps from zero position
        steps = filter_pos - self.filter_position_now
        self.filter_position_now = filter_pos

        self.move_filter(steps=steps, speed=speed, filter_axis=filter_axis, is_blocking=is_blocking, timeout=timeout, is_enabled=is_enabled)


    def move_filter(self, steps=100, speed=200, filter_axis=-1, timeout=10, is_enabled=False, is_blocking=False):
        steps_xyzt = np.zeros(4)
        steps_xyzt[filter_axis] = steps
        r = self.move_stepper(steps=steps_xyzt, speed=speed, timeout=timeout, is_enabled=is_enabled, is_blocking=is_blocking)
        return r

    def move_forever(self, speed=(0,0,0,0), is_stop=False):
        if type(speed)==int:
            speed=(speed, speed, speed, speed)
        if len(speed)==3:
            speed = (*speed,0)
        path = "/motor_act"

        payload = {
            "task":path,
            "motor":
            {
                "steppers": [
                    { "stepperid": 0, "speed": speed[0], "isforver":1, "isstop":is_stop},
                    { "stepperid": 1, "speed": speed[1], "isforver":1, "isstop":is_stop},
                    { "stepperid": 2, "speed": speed[2], "isforver":1, "isstop":is_stop},
                    { "stepperid": 3, "speed": speed[3], "isforver":1, "isstop":is_stop},
                ]
            }
        }
        r = self._parent.post_json(path, payload, timeout=0)
        return r

    def move_stepper(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), is_absolute=(False,False,False,False), timeout=1, backlash=(0,0,0,0), is_blocking=True, is_enabled=True):
        '''
        This tells the motor to run at a given speed for a specific number of steps; Multiple motors can run simultaneously

        XYZT => 1,2,3,0
        '''
        if type(is_absolute)==bool:
            is_absolute = (is_absolute,is_absolute,is_absolute,is_absolute)

        # convert single elements to array
        if type(speed)!=list and type(speed)!=tuple and type(speed)!=np.ndarray:
            speed = np.array((speed,speed,speed,speed))

        # make sure value is an array
        if type(steps)==tuple:
            steps = np.array(steps)

        # detect change in directiongit config --global user.name "Your Name"
        for iMotor in range(4):
            if np.sign(self.steps_last[iMotor]) != np.sign(steps[iMotor]):
                # we want to overshoot a bit
                steps[iMotor] = steps[iMotor] + (np.sign(steps[iMotor])*backlash[iMotor])


        '''
        # get current position
        pos_0 = self.get_position(0)
        pos_1 = self.get_position(1)
        pos_2 = self.get_position(2)
        pos_3 = self.get_position(3)

        # convert to physical units
        steps_0 = steps_0*self.stepSizeX
        steps_1 = steps_1*self.stepSizeY
        steps_2 = steps_2*self.stepSizeZ
        steps_3 = steps_3*self.stepSizeT

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

        '''
        {"task":"/motor_act",
        "motor":{
            "steppers":[
                {
                    "stepperid":1,
                    "position":-1000,
                    "speed":2000,
                    "isabs":0,
                    "isaccel":0
                }
            ]
        }
        }

        {"task":"/motor_act",
    "motor":
    {
        "steppers": [
            { "stepperid": 1, "position": -100, "speed": 2000, "isabs": 0, "isaccel":0}
        ]
    }
}
        '''
        
        # only consider those actions that are necessary
        motorPropList = []
        for iMotor in range(4):
            if is_absolute[iMotor] or abs(steps[iMotor])>0:
                motorProp = { "stepperid": iMotor, "position": np.int(steps[iMotor]), "speed": speed[iMotor], "isabs": is_absolute[iMotor], "isaccel":0}
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
        r = self._parent.post_json(path, payload, getReturn=False, timeout=1)

        # wait until job has been done
        time0=time.time()

        steppersRunning = np.array(steps)>0
        if is_blocking and self._parent.serial.is_connected:
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
                    try:
                        rMessage = rMessage.split("\r")[0].replace("'", '"')
                        mMessage = json.loads(rMessage)
                        for iElement in mMessage['steppers']: 
                            if iElement['isDone']:
                                mNumber = iElement['stepperid']
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
                        {   "stepperid": stepperid}]
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

    def set_motor_acceleration(self, axis=0, acceleration=10000):
        if type(axis)==str:
            axis = self.xyztTo1230(axis)

        r = self.set_motor(stepperid = axis, acceleration=acceleration)
        return r

    def set_motor_enable(self, axis =0, is_enable=1):
        if type(axis)==str:
            axis = self.xyztTo1230(axis)

        r = self.set_motor(stepperid = axis, isEnable=is_enable)
        return r

    def get_position(self, timeout=1):
        path = "/motor_get"
        payload = {
            "task":path,
            "position":1,
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        _position = np.array((0,0,0,0))
        try:
            for istepper in r["motor"]["steppers"]:
                _position[istepper["stepperid"]]=istepper["position"]
        except Exception as e: self._parent.logger.error(e)
        return _position

    def set_position(self, axis=1, position=0, timeout=1):
        '''
        path = "/motor_set"
        if axis=="X": axis=1
        if axis=="Y": axis=2
        if axis=="Z": axis=3

        payload = {
            "task":path,
            "axis":axis,
            "currentposition": position
        }
        r = self._parent.post_json(path, payload, timeout=timeout)

        return r
        '''
        return False

    def set_motors(self, settingsdict, isBlocking=True, timeout=1):
        path = "/motor_set"
        payload = settingsdict
        payload["task"] = path

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


    def get_motor(self, axis=1, timeout=1):
        path = "/motor_get"
        payload = {
            "task":path,
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try: return r["steppers"][axis]
        except: return False

    def get_motors(self, timeout=1):
        path = "/motor_get"
        payload = {
            "task":path,
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try: return r["steppers"]
        except: return False


    def set_direction(self, axis=1, sign=1, timeout=1):
        return False
