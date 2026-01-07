import time
import json      
import numpy as np  
        
class Home(object):
    ## Laser
    def __init__(self, parent):
        self._parent = parent
        self.endposrelease = 3000
        self.direction = 1
        self.speed = 15000
        self.timeout = 20000
        self.endstoppolarity = 1
        
        self.nMotors = 4
        self.isHomed = np.zeros((self.nMotors))
        

        # register a callback function for the motor status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_home_status, pattern="home")
        
        # announce a function that is called when we receive a position update through the callback
        self._callbackPerKey = {}
        self.nCallbacks = 10
        self._callbackPerKey = self.init_callback_functions(nCallbacks=self.nCallbacks) # only one is used for now
        print(self._callbackPerKey)
        

    def init_callback_functions(self, nCallbacks=10):
        ''' initialize the callback functions '''
        _callbackPerKey = {}
        self.nCallbacks = nCallbacks
        for i in range(nCallbacks):
            _callbackPerKey[i] = []
        return _callbackPerKey
            
    def _callback_home_status(self, data):
        ''' cast the json in the form:
        ++
        {"home":{"steppers":[{"axis":1,"pos":0,"isDone":1}]},"qid":0}
        --
        into the position array of the motors '''
        try:
            steppers = data["home"]["steppers"]
            # Handle both single stepper and multiple steppers
            if isinstance(steppers, dict):
                # Single stepper format: {"axis":1,"pos":0,"isDone":1}
                stepperID = steppers["axis"]
                self.isHomed[stepperID] = steppers["isDone"]
            elif isinstance(steppers, list):
                # Multiple steppers format: [{"axis":1,"pos":0,"isDone":1}, ...]
                for stepper in steppers:
                    stepperID = stepper["axis"]
                    self.isHomed[stepperID] = stepper["isDone"]
            
            if callable(self._callbackPerKey[0]):
                self._callbackPerKey[0](self.isHomed) # we call the function with the value
        except Exception as e:
            print("Error in _callback_home_status: ", e)

    def register_callback(self, key, callbackfct):
        ''' register a callback function for a specific key '''
        self._callbackPerKey[key] = callbackfct
        
                
    def home_x(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axis = 1 corresponds to 'X'
        axis = 1
        self.home(axis=axis, 
                  endstoptimeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease,
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)

    def home_y(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axis = 2 corresponds to 'Y'
        axis = 2
        self.home(axis=axis, 
                  endstoptimeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease, 
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)    
    
    def home_z(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axisa = 3 corresponds to 'Z'
        axis = 3
        self.home(axis=axis, 
                  endstoptimeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease, 
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)    
        
    def home_a(self, speed = None, direction = None, endposrelease = None, endstoppolarity=None, timeout=None, isBlocking=False):
        # axis = 0 corresponds to 'A'
        axis = 0
        self.home(axis=axis,
                  endstoptimeout=timeout, 
                  speed = speed, 
                  direction = direction, 
                  endposrelease=endposrelease, 
                  endstoppolarity=endstoppolarity,
                  isBlocking=isBlocking)    

    def home_xy(self, axes=["X", "Y"], speeds=None, directions=None, endposreleases=None, 
                endstoppolarities=None, timeouts=None, isBlocking=False, preMove=True):
        """
        Home multiple axes simultaneously.
        
        :param axes: List of axes to home (e.g., ["X", "Y"] or [1, 2])
        :param speeds: List of speeds for each axis (or single speed for all)
        :param directions: List of directions for each axis (or single direction for all)
        :param endposreleases: List of endpos release values for each axis (or single value for all)
        :param endstoppolarities: List of endstop polarities for each axis (or single value for all)
        :param timeouts: List of timeouts for each axis (or single timeout for all)
        :param isBlocking: If True, wait for all motors to finish homing
        :param preMove: If True, move away from endstop before homing
        :return: Response from the device
        
        Example:
            # Home X and Y axes with different parameters
            ESP32.home.home_xy(
                axes=["X", "Y"],
                speeds=[15000, 15000],
                directions=[-1, 1],
                timeouts=[400, 200]
            )
        """
        # Convert axes to stepper IDs
        # First get logical motor index, then map through motorAxisOrder to get physical stepper ID
        stepper_ids = []
        motorAxisOrder = self._parent.motor.motorAxisOrder if hasattr(self._parent.motor   , 'motorAxisOrder') else [0, 1, 2, 3]
        for axis in axes:
            if isinstance(axis, str):
                logical_motor_index = self.xyztTo1230(axis)
            else:
                logical_motor_index = axis
            # Map logical motor index to physical stepper ID
            stepper_id = motorAxisOrder[logical_motor_index]
            stepper_ids.append(stepper_id)
        
        num_motors = len(stepper_ids)
        
        # Handle parameter lists - convert single values to lists
        def ensure_list(param, default_value):
            if param is None:
                return [default_value] * num_motors
            elif not isinstance(param, list):
                return [param] * num_motors
            elif len(param) == 1:
                return param * num_motors
            else:
                return param[:num_motors]  # Truncate if too long
        
        # Use internal parameters as defaults
        speeds = ensure_list(speeds, self.speed)
        directions = ensure_list(directions, self.direction)
        endposreleases = ensure_list(endposreleases, self.endposrelease)
        endstoppolarities = ensure_list(endstoppolarities, self.endstoppolarity)
        timeouts = ensure_list(timeouts, self.timeout)
        
        # Validate directions
        for i, direction in enumerate(directions):
            if direction not in [-1, 1]:
                directions[i] = 1
        
        # Pre-move if requested
        if preMove:
            print("Pre-moving motors away from endstops...")
            for i, stepper_id in enumerate(stepper_ids):
                # Move in opposite direction
                preMoveDirection = -directions[i]
                preMoveDistanceSteps = 100  # steps to move away from endstop
                
                # Move away from endstop using the motor controller
                if stepper_id == 1:  # X
                    self._parent.motor.move_x(steps=preMoveDirection*preMoveDistanceSteps, 
                                            speed=speeds[i], is_blocking=True, is_absolute=False, is_enabled=True)
                elif stepper_id == 2:  # Y
                    self._parent.motor.move_y(steps=preMoveDirection*preMoveDistanceSteps, 
                                            speed=speeds[i], is_blocking=True, is_absolute=False, is_enabled=True)
                elif stepper_id == 3:  # Z
                    self._parent.motor.move_z(steps=preMoveDirection*preMoveDistanceSteps, 
                                            speed=speeds[i], is_blocking=True, is_absolute=False, is_enabled=True)
                elif stepper_id == 0:  # A
                    self._parent.motor.move_a(steps=preMoveDirection*preMoveDistanceSteps, 
                                            speed=speeds[i], is_blocking=True, is_absolute=False, is_enabled=True)
            time.sleep(0.5)
        
        # Build the steppers list for the JSON payload
        steppers_list = []
        for i, stepper_id in enumerate(stepper_ids):
            stepper_config = {
                "stepperid": stepper_id,
                "timeout": timeouts[i],
                "speed": abs(speeds[i]),
                "direction": directions[i],
                "endstoppolarity": endstoppolarities[i]
            }
            steppers_list.append(stepper_config)
        
        # Construct JSON payload
        path = "/home_act"
        payload = {
            "task": path,
            "home": {
                "steppers": steppers_list
            }
        }
        
        # Calculate number of expected responses
        # One response for command acknowledgment + one response per motor when done
        nResponses = 1 + num_motors if isBlocking else 0
        response_timeout = max(timeouts) / 1000 + 5 if isBlocking else 0  # Convert ms to seconds + buffer
        
        # Send the command
        r = self._parent.post_json(path, payload, getReturn=isBlocking, 
                                 timeout=response_timeout, nResponses=nResponses)
        
        return r
    
    def xyztTo1230(self, axis):
        """Convert axis string to stepper ID number."""
        if isinstance(axis, str):
            axis = axis.upper()
            if axis == "X":
                return 1
            elif axis == "Y":
                return 2
            elif axis == "Z":
                return 3
            elif axis == "A":
                return 0
        return axis
    
    def home(self, axis=None, timeout=None, speed=None, direction=None, endposrelease=None, endstoppolarity=None, endstoptimeout=10000, isBlocking=False, preMove=True):
        '''
        axis = 0,1,2,3 or 'A, 'X','Y','Z'
        timeout => when to stop homing (it's a while loop on the MCU)
        speed => speed of homing (0...15000)
        direction => 1,-1 (left/right)
        endposrelease => how far to move after homing (0...3000)
        preMove => the motor will first move by some steps in the opposite direction before homing, this is useful to avoid false triggering of the endstop
        '''
        
        # default values
        if speed is None:
            speed = self.speed
        if direction is None:
            direction = self.direction
        if endposrelease is None:
            endposrelease = self.endposrelease
        if timeout is None:
            timeout = self.timeout
        if endstoppolarity is None:
            endstoppolarity = self.endstoppolarity

        if direction not in [-1,1]:
            direction = 1

        if preMove:
            # first move in the opposite direction
            if direction == 1:
                preMoveDirection = -1
            else:
                preMoveDirection = 1
            preMoveDistanceSteps = 100 # steps to move away from endstop
            
            # move away from endstop
            if axis == 1 or axis == "X":
                self._parent.motor.move_x(steps=preMoveDirection*preMoveDistanceSteps, speed=self.speed, is_blocking=True, is_absolute=False, is_enabled=True)
            elif axis == 2 or axis == "Y":
                self._parent.motor.move_y(steps=preMoveDirection*preMoveDistanceSteps, speed=self.speed, is_blocking=True, is_absolute=False, is_enabled=True)
            elif axis == 3 or axis == "Z":
                self._parent.motor.move_z(steps=preMoveDirection*preMoveDistanceSteps, speed=self.speed, is_blocking=True, is_absolute=False, is_enabled=True)
            elif axis == 0 or axis == "A":
                self._parent.motor.move_a(steps=preMoveDirection*preMoveDistanceSteps, speed=self.speed, is_blocking=True, is_absolute=False, is_enabled=True)
            else:   
                raise ValueError("Invalid axis. Use 'X', 'Y', 'Z', or 'A'.")
            time.sleep(0.5)
        
        # Map logical motor index to physical stepper ID through motorAxisOrder
        stepper_id = self._parent.motor.motorAxisOrder[axis]
        
        # construct json string
        path = "/home_act"

        payload = {
            "task": path,
            "home":{
                "steppers": [
                {
                 "stepperid": stepper_id,
                 "timeout":endstoptimeout,
                 "speed":abs(speed),
                 "direction":direction,
                 "endstoppolarity":endstoppolarity
                 }]
            }}
     
        timeout = timeout if isBlocking else 0
        nResponses = 2 # one for command received, one for home reached
        
        # if we get a return, we will receive the latest position feedback from the driver  by means of the axis that moves the longest
        r = self._parent.post_json(path, payload, getReturn=isBlocking, timeout=timeout, nResponses=nResponses)

        return r

