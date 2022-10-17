import numpy as np
import time 

class Motor(object):
    
    microsteppingfactor_filter=16 # run more smoothly
    filter_pos_1 = 1000*microsteppingfactor_filter # GFP
    filter_pos_2 = 0*microsteppingfactor_filter # AF647/SIR
    filter_pos_3 = 500*microsteppingfactor_filter
    filter_pos_LED = filter_pos_1 # GFP / Brightfield
    filter_pos_init = -1250*microsteppingfactor_filter
    filter_speed = microsteppingfactor_filter * 500
    filter_position_now = 0


    def __init__(self, parent=None):
        self.steps_last_0 = 0
        self.steps_last_1 = 0
        self.steps_last_2 = 0
        self.steps_last_3 = 0
        
        self.backlash_x = 0
        self.backlash_y = 0
        self.backlash_z = 0
        self.backlash_t = 0

        self._parent = parent
        
        
    '''################################################################################################################################################
    HIGH-LEVEL Functions that rely on basic REST-API functions
    ################################################################################################################################################'''

    def move_x(self, steps=100, speed=1000, is_blocking=False, is_absolute=False, is_enabled=True):
        r = self.move_stepper(steps=(steps,0,0,0), speed=speed, timeout=1, backlash=(self.backlash_x,0,0,0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled)
        return r

    def move_y(self, steps=100, speed=1000, is_blocking=False, is_absolute=False, is_enabled=True):
        r = self.move_stepper(steps=(0,steps,0,0), speed=speed, timeout=1, backlash=(0,self.backlash_y,0,0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled)
        return r

    def move_z(self, steps=100, speed=1000, is_blocking=False, is_absolute=False, is_enabled=True):
        r = self.move_stepper(steps=(0,0,steps,0), speed=speed, timeout=1, backlash=(0,0,self.backlash_z,0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled)
        return r

    def move_t(self, steps=100, speed=1000, is_blocking=False, is_absolute=False, is_enabled=True):
        r = self.move_stepper(steps=(0,0,0,steps), speed=speed, timeout=1, backlash=(0,0,self.backlash_z,0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled)
        return r
    
    def move_xyz(self, steps=(0,0,0), speed=(1000,1000,1000), is_blocking=False, is_absolute=False, is_enabled=True):
        if len(speed)!= 3:
            speed = (speed,speed,speed)

        r = self.move_xyzt(steps=(steps[0],steps[1],steps[2],0), speed=(speed[0],speed[1],speed[2],0), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled)
        return r

    def move_xyzt(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), is_blocking=False, is_absolute=False, is_enabled=True):
        if type(speed)==int:
            speed = (speed,speed,speed,speed)
        if type(steps)==int:
            steps = (steps,steps,steps,steps)
        

        r = self.move_stepper(steps=steps, speed=speed, timeout=1, backlash=(self.backlash_x,self.backlash_y,self.backlash_z,self.backlash_t), is_blocking=is_blocking, is_absolute=is_absolute, is_enabled=is_enabled)
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




    def move_forever(self, speed=(0,0,0), is_stop=False):
        path = "/motor_act"
        payload = {
            "task":path,
            "speed0": np.int(speed[0]), # TODO: need a fourth axis?
            "speed1": np.int(speed[0]),
            "speed2": np.int(speed[1]),
            "speed3": np.int(speed[2]),
            "isforever":1,
            "isaccel":1,
            "isstop": np.int(is_stop)
        }
        # Make sure PS controller is treated correclty
        #if self.isControllerMode():
        #    self.setControllerMode(isController=False)
        #    PSwasActive = True
        #else:
        #    PSwasActive = False

        r = self._parent.post_json(path, payload, timeout=0)

        #if PSwasActive:
        #    self.setControllerMode(isController=True)

        return r

    def move_stepper(self, steps=(0,0,0,0), speed=(1000,1000,1000,1000), is_absolute=False, timeout=1, backlash=(0,0,0,0), is_blocking=True, is_enabled=True):
        '''
        This tells the motor to run at a given speed for a specific number of steps; Multiple motors can run simultaneously
        '''
        if type(speed)!=list and type(speed)!=tuple  :
            speed = (speed,speed,speed,speed)

        path = "/motor_act"

        # detect change in directiongit config --global user.name "Your Name"
        if np.sign(self.steps_last_0) != np.sign(steps[0]):
            # we want to overshoot a bit
            steps_0 = steps[0] + (np.sign(steps[0])*backlash[0])
        else: steps_0 = steps[0]
        if np.sign(self.steps_last_1) != np.sign(steps[1]):
            # we want to overshoot a bit
            steps_1 =  steps[1] + (np.sign(steps[1])*backlash[1])
        else: steps_1 = steps[1]
        if np.sign(self.steps_last_2) != np.sign(steps[2]):
            # we want to overshoot a bit
            steps_2 =  steps[2] + (np.sign(steps[2])*backlash[2])
        else: steps_2 = steps[2]
        if np.sign(self.steps_last_3) != np.sign(steps[3]):
            # we want to overshoot a bit
            steps_3 =  steps[3] + (np.sign(steps[3])*backlash[3])
        else: steps_3 = steps[3]

        payload = {
            "task":"/motor_act",
            "pos0": np.int(steps_3),
            "pos1": np.int(steps_0),
            "pos2": np.int(steps_1),
            "pos3": np.int(steps_2),
            "isblock": int(is_blocking),
            "isabs": int(is_absolute),
            "speed0": np.int(speed[3]),
            "speed1": np.int(speed[0]),
            "speed2": np.int(speed[1]),
            "speed3": np.int(speed[2]),
            "isen": np.int(is_enabled)
        }
        self.steps_last_0 = steps_0
        self.steps_last_1 = steps_1
        self.steps_last_2 = steps_2
        self.steps_last_3 = steps_3

        # drive motor
        r = self._parent.post_json(path, payload, timeout=timeout)

        #if PSwasActive:
        #    self.setControllerMode(isController=True)
        # wait until job has been done
        time0=time.time()
        if is_blocking:
            while self._parent.state.isBusy():
                time.sleep(0.1)
                if time.time()-time0>timeout:
                    break

        return r


    def set_motor_maxSpeed(self, axis=0, maxSpeed=10000):
        path = "/motor_set",
        payload = {
            "task": path,
            "axis": axis,
            "maxspeed": maxSpeed
        }
        r = self._parent.post_json(path, payload)
        return r

    def set_motor_currentPosition(self, axis=0, currentPosition=10000):
        path = "/motor_set",
        payload = {
            "task": path,
            "axis": axis,
            "currentposition": currentPosition
        }
        r = self._parent.post_json(path, payload)
        return r

    def set_motor_acceleration(self, axis=0, acceleration=10000):
        path = "/motor_set",
        payload = {
            "task": path,
            "axis": axis,
            "acceleration": acceleration
        }
        r = self._parent.post_json(path, payload)
        return r


    def set_motor_enable(self, is_enable=1):
        path = "/motor_set",
        payload = {
            "task": path,
            "isen": is_enable
        }
        r = self._parent.post_json(path, payload)
        return r

    def set_direction(self, axis=1, sign=1, timeout=1):
        path = "/motor_set"

        payload = {
            "task":path,
            "axis": axis,
            "sign": sign
        }

        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def get_position(self, axis=1, timeout=1):
        path = "/motor_get"

        payload = {
            "task":path,
            "axis": axis
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try: _position = r["position"]
        except: _position = None
        return _position

    def set_position(self, axis=1, position=0, timeout=1):
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

