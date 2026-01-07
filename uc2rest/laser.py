import numpy as np
class Laser(object):
    ## Laser
    def __init__(self, parent):
        self._parent = parent
        self.filter_pos_1 = 0
        self.filter_pos_2 = 0
        self.filter_pos_3 = 0
        self.filter_pos_LED = 0
        
        self.nLasers = 4
        self.laserValues = np.zeros((self.nLasers))
                
        # register a callback function for the laser status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_laser_status, pattern="laser")
        
        # announce a function that is called when we receive a laser update through the callback
        self._callbackPerKey = {}
        self.nCallbacks = 10
        self._callbackPerKey = self.init_callback_functions(nCallbacks=self.nCallbacks) # only one is used for now
        print(self._callbackPerKey)
        
      
    def init_callback_functions(self, nCallbacks=10):
        ''' initialize the callback functions - each key holds a list of callbacks '''
        _callbackPerKey = {}
        self.nCallbacks = nCallbacks
        for i in range(nCallbacks):
            _callbackPerKey[i] = []  # Initialize as list to support multiple callbacks
        return _callbackPerKey
            
    def _callback_laser_status(self, data):
        ''' cast the json in the form:
        ++
        {"laser":{"LASERid":1,"LASERval":512,"isDone":1},"qid":2}
        --
        into the laser values array '''
        try:
            laser_data = data["laser"]
            # Handle both single laser and multiple lasers
            if isinstance(laser_data, dict):
                # Single laser format: {"LASERid":1,"LASERval":512,"isDone":1}
                laser_id = laser_data.get("LASERid", 0)
                laser_val = laser_data.get("LASERval", 0)
                if 0 <= laser_id < self.nLasers:
                    self.laserValues[laser_id] = laser_val
            elif isinstance(laser_data, list):
                # Multiple lasers format: [{"LASERid":1,"LASERval":512,"isDone":1}, ...]
                for laser in laser_data:
                    laser_id = laser.get("LASERid", 0)
                    laser_val = laser.get("LASERval", 0)
                    if 0 <= laser_id < self.nLasers:
                        self.laserValues[laser_id] = laser_val
            
            # Call all registered callbacks for key 0
            for callback in self._callbackPerKey[0]:
                if callable(callback):
                    try:
                        callback(self.laserValues)
                    except Exception as callback_error:
                        print(f"Error in callback execution: {callback_error}")
        except Exception as e:
            print("Error in _callback_laser_status: ", e)

    def register_callback(self, key, callbackfct):
        ''' register a callback function for a specific key - supports multiple callbacks per key '''
        if key not in self._callbackPerKey:
            self._callbackPerKey[key] = []
        if callbackfct not in self._callbackPerKey[key]:  # Avoid duplicate registrations
            self._callbackPerKey[key].append(callbackfct)
            print(f"Registered callback for key {key}. Total callbacks for this key: {len(self._callbackPerKey[key])}")
        

  
    def set_laser(self, channel=1, value=0, auto_filterswitch=False,
                        filter_axis=-1, filter_position = None,
                        despeckleAmplitude = 0.,
                        despecklePeriod=10, timeout=20, is_blocking = False):
        if channel not in (0,1,2,3):
            if channel=="R":
                channel = 1
            elif channel=="G":
                channel = 2
            elif channel=="B":
                channel = 3

        if auto_filterswitch and value >0:
            if filter_position is None:
                if channel==1:
                    filter_position_toGo = self.filter_pos_1
                if channel==2:
                    filter_position_toGo = self.filter_pos_2
                if channel==3:
                    filter_position_toGo = self.filter_pos_3
                if channel=="LED":
                    filter_position_toGo = self.filter_pos_LED
            else:
                filter_position_toGo = filter_position

            self.switch_filter(filter_pos=filter_position_toGo, filter_axis=filter_axis, timeout=timeout,is_blocking=is_blocking)

        path = '/laser_act'
        
        payload = {
            "task": path,
            "LASERid": channel,
            "LASERval": value,
            "LASERdespeckle": int(value*despeckleAmplitude),
            "LASERdespecklePeriod": int(despecklePeriod),

        }
        #self._parent.logger.debug("Setting Laser "+str(channel)+", value: "+str(value))
        r = self._parent.post_json(path, payload, getReturn=is_blocking, timeout=0.5)
        return r

    def set_laserpin(self, laserid=1, laserpin=0):
        path = '/laser_set'
        
        payload = {
            "task": path,
            "LASERid": laserid,
            "LASERpin": laserpin
        }
        
        r = self._parent.post_json(path, payload)
        return r



    def set_servo(self, channel=1, value=0, is_blocking=False):
        #{"task":"/laser_act", "LASERid":1 ,"LASERval":99, "servo":1, "qid":1}
        path = '/laser_act'
        
        payload = {
            "task": path,
            "LASERid": channel,
            "LASERval": value, 
            "servo": 1
        }
        
        r = self._parent.post_json(path, payload, getReturn=is_blocking)
        return r