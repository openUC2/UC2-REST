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
            # single update {"LASERid":..}, a list of them, or the /laser_get answer {"lasers":[..]}
            lasers = laser_data if isinstance(laser_data, list) else laser_data.get("lasers", [laser_data])
            for laser in lasers:
                laser_id = laser.get("LASERid", -1)
                if "LASERval" in laser and 0 <= laser_id < self.nLasers:
                    self.laserValues[laser_id] = laser["LASERval"]
            
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

    def get_laser(self, channel=None, timeout=1):
        """Laser parameters from /laser_get, one dict per laser, e.g.
        {"LASERid":1,"LASERpin":12,"LASERval":0,"LASERmax":1024,"LASERRes":10,"LASERFreq":5000,
         "LASERdespeckle":0,"LASERdespecklePeriod":20}
        CAN lasers carry "node" (and "error" if the slave did not answer).
        channel None: {LASERid: params} for all lasers; an id: that laser's dict or None.
        Firmware without the "lasers" list yields {} / None.
        """
        r = self._parent.post_json("/laser_get", {"task": "/laser_get"}, getReturn=True, nResponses=1, timeout=timeout)
        if isinstance(r, list):
            r = next((i for i in r if isinstance(i, dict) and "laser" in i), {})
        laser = r.get("laser", {}) if isinstance(r, dict) else {}
        lasers = {l["LASERid"]: l for l in laser.get("lasers", []) if "LASERid" in l}
        return lasers if channel is None else lasers.get(channel)

    def get_laser_max(self, channel=1, timeout=1):
        """Maximum PWM value of a laser (2**resolution, i.e. fully on), None if unknown."""
        return (self.get_laser(channel, timeout=timeout) or {}).get("LASERmax")

    def set_laser_pwm(self, channel=1, frequency=None, resolution=None, value=0, timeout=1):
        """Change PWM frequency (Hz) and/or resolution (bits) of a laser; LASERmax becomes 2**resolution.
        The firmware needs a LASERval with it, so the laser is set to `value` (default off) afterwards.
        Only on-board lasers: CAN slaves ignore LASERFreq/LASERRes.
        """
        payload = {"task": "/laser_act", "LASERid": channel, "LASERval": int(value)}
        if frequency is not None:
            payload["LASERFreq"] = int(frequency)
        if resolution is not None:
            payload["LASERRes"] = int(resolution)
        return self._parent.post_json("/laser_act", payload, timeout=timeout)

    def set_strobe(self, channel, enable=True, delay_us=0, width_us=20, timeout=2.0):
        """Configure the strobe of a logical laser id (one flash per strobed-sweep frame).

        {"task":"/laser_act","LASERid":channel,"strobe":{"enable":1,"delayUs":..,"widthUs":..}}
        Returns the firmware answer, e.g.
        {"strobe":{"supported":1,"enabled":1,"remote":1,"node":30,"delayUs":1200,"widthUs":20,"count":0},"return":1}
        or {"strobe":{"supported":0,...},"return":0,"error":"..."}; None when the
        firmware did not answer (older firmware).
        """
        path = "/laser_act"
        payload = {
            "task": path,
            "LASERid": int(channel),
            "strobe": {"enable": 1 if enable else 0, "delayUs": int(delay_us), "widthUs": int(width_us)},
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        if isinstance(r, list):
            for item in r:
                if isinstance(item, dict) and "strobe" in item:
                    return item
        if isinstance(r, dict) and "strobe" in r:
            return r
        return None

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