import numpy as np

class DigitalOut(object):
    ## DigitalOut
    def __init__(self, parent):
        self._parent = parent
        self._logger = parent.logger
        self.nDigitalOuts = 3
        self.digitalOutValues = np.zeros((self.nDigitalOuts), dtype=int)
                
        # register a callback function for the digitalout status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_digitalout_status, pattern="digitalout")
        
        # announce a function that is called when we receive a digitalout update through the callback
        self._callbackPerKey = {}
        self.nCallbacks = 10
        self._callbackPerKey = self.init_callback_functions(nCallbacks=self.nCallbacks)
        print(self._callbackPerKey)
        
      
    def init_callback_functions(self, nCallbacks=10):
        ''' initialize the callback functions - each key holds a list of callbacks '''
        _callbackPerKey = {}
        self.nCallbacks = nCallbacks
        for i in range(nCallbacks):
            _callbackPerKey[i] = []  # Initialize as list to support multiple callbacks
        return _callbackPerKey
            
    def _callback_digitalout_status(self, data):
        ''' cast the json in the form:
        ++
        {"digitalout":{"digitaloutid":1,"digitaloutval":1,"digitaloutpin":4,"isDone":1},"qid":2}
        --
        into the digitalout values array '''
        try:
            digitalout_data = data["digitalout"]
            # Handle both single digitalout and multiple digitalouts
            if isinstance(digitalout_data, dict):
                # Single digitalout format: {"digitaloutid":1,"digitaloutval":1,"digitaloutpin":4}
                digitalout_id = digitalout_data.get("digitaloutid", 0)
                digitalout_val = digitalout_data.get("digitaloutval", 0)
                if 0 < digitalout_id <= self.nDigitalOuts:
                    self.digitalOutValues[digitalout_id - 1] = digitalout_val
            elif isinstance(digitalout_data, list):
                # Multiple digitalouts format: [{"digitaloutid":1,"digitaloutval":1}, ...]
                for digitalout in digitalout_data:
                    digitalout_id = digitalout.get("digitaloutid", 0)
                    digitalout_val = digitalout.get("digitaloutval", 0)
                    if 0 < digitalout_id <= self.nDigitalOuts:
                        self.digitalOutValues[digitalout_id - 1] = digitalout_val
            
            # Call all registered callbacks for key 0
            for callback in self._callbackPerKey[0]:
                if callable(callback):
                    try:
                        callback(self.digitalOutValues)
                    except Exception as callback_error:
                        print(f"Error in callback execution: {callback_error}")
        except Exception as e:
            print("Error in _callback_digitalout_status: ", e)

    def register_callback(self, key, callbackfct):
        ''' register a callback function for a specific key - supports multiple callbacks per key '''
        if key not in self._callbackPerKey:
            self._callbackPerKey[key] = []
        if callbackfct not in self._callbackPerKey[key]:  # Avoid duplicate registrations
            self._callbackPerKey[key].append(callbackfct)
            print(f"Registered callback for key {key}. Total callbacks for this key: {len(self._callbackPerKey[key])}")
    
    def setup_digitaloutpin(self, id=1, pin=4):
        path = '/digitalout_set'
        payload = {
            "task": path,
            "digitaloutid": id,
            "digitaloutpin": pin,
        }
        r = self._parent.post_json(path, payload)
        return r
    
    def reset_triggertable(self):
        path = '/digitalout_act'
        payload = {
            "task": path,
            "digitaloutistriggerreset": 1,
        }
        r = self._parent.post_json(path, payload)
        return r
    
    def set_trigger(self, trigger1=False, delayOn1=0, delayOff1=0, trigger2=False, delayOn2=0, delayOff2=0, trigger3=False, delayOn3=0, delayOff3=0):
        '''
        this stats a trigger table with 3 triggers
        
        Parameters:
        
        '''
        path = '/digitalout_act'
        
        payload = {
            "task":path,
            "digitalout1TriggerDelayOn":delayOn1,
            "digitalout1TriggerDelayOff":delayOff1,
            "digitalout1IsTrigger":trigger1,
            "digitalout2TriggerDelayOn":delayOn2,
            "digitalout2TriggerDelayOff":delayOff2,
            "digitalout2IsTrigger":trigger2, 
            "digitalout3TriggerDelayOn":delayOn3,
            "digitalout3TriggerDelayOff":delayOff3,
            "digitalout3IsTrigger":trigger3,
        }
        
        r = self._parent.post_json(path, payload)
        return r
    
        


    def sendTrigger(self, triggerId=0):
        path = '/digital_act'

        payload = {
            "task": path,
            "digitalid": triggerId,
            "digitalval": -1,
        }

        r = self._parent.post_json(path, payload)
        return r

    def get_digitalout(self, digitaloutid=1, timeout=1, is_blocking=True):
        """
        Get the current value and pin of a digital output.
        
        Parameters:
        -----------
        digitaloutid : int
            ID of the digital output (1, 2, or 3)
        timeout : float
            Timeout for the request in seconds
        is_blocking : bool
            Whether to wait for a response
            
        Returns:
        --------
        dict or None
            Response from the device containing digitalout status (id, val, pin)
        """
        path = '/digitalout_get'
        
        payload = {
            "task": path,
            "digitaloutid": digitaloutid
        }
        
        r = self._parent.post_json(path, payload, getReturn=is_blocking, timeout=timeout)
        return r

    def set_digitalout(self, digitaloutid=1, digitaloutval=0, timeout=1, is_blocking=False):
        """
        Set the value of a digital output. Use digitaloutval=-1 to trigger a pulse (HIGH->LOW).
        
        Parameters:
        -----------
        digitaloutid : int
            ID of the digital output (1, 2, or 3)
        digitaloutval : int
            Value to set (0=LOW, 1=HIGH, -1=pulse/trigger)
        timeout : float
            Timeout for the request in seconds
        is_blocking : bool
            Whether to wait for a response
            
        Returns:
        --------
        dict or None
            Response from the device
        """
        path = '/digitalout_act'
        
        payload = {
            "task": path,
            "digitaloutid": digitaloutid,
            "digitaloutval": digitaloutval
        }
        
        r = self._parent.post_json(path, payload, getReturn=is_blocking, timeout=timeout)
        return r
