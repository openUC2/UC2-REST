import numpy as np

class DigitalIn(object):
    ## DigitalIn
    def __init__(self, parent):
        self._parent = parent
        self.nDigitalIns = 3
        self.digitalInValues = np.zeros((self.nDigitalIns), dtype=int)
                
        # register a callback function for the digitalin status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_digitalin_status, pattern="digitalin")
        
        # announce a function that is called when we receive a digitalin update through the callback
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
            
    def _callback_digitalin_status(self, data):
        ''' cast the json in the form:
        ++
        {"digitalin":{"digitalinid":1,"digitalinval":1,"isDone":1},"qid":2}
        --
        into the digitalin values array '''
        try:
            digitalin_data = data["digitalin"]
            # Handle both single digitalin and multiple digitalins
            if isinstance(digitalin_data, dict):
                # Single digitalin format: {"digitalinid":1,"digitalinval":1}
                digitalin_id = digitalin_data.get("digitalinid", 0)
                digitalin_val = digitalin_data.get("digitalinval", 0)
                if 0 < digitalin_id <= self.nDigitalIns:
                    self.digitalInValues[digitalin_id - 1] = digitalin_val
            elif isinstance(digitalin_data, list):
                # Multiple digitalins format: [{"digitalinid":1,"digitalinval":1}, ...]
                for digitalin in digitalin_data:
                    digitalin_id = digitalin.get("digitalinid", 0)
                    digitalin_val = digitalin.get("digitalinval", 0)
                    if 0 < digitalin_id <= self.nDigitalIns:
                        self.digitalInValues[digitalin_id - 1] = digitalin_val
            
            # Call all registered callbacks for key 0
            for callback in self._callbackPerKey[0]:
                if callable(callback):
                    try:
                        callback(self.digitalInValues)
                    except Exception as callback_error:
                        print(f"Error in callback execution: {callback_error}")
        except Exception as e:
            print("Error in _callback_digitalin_status: ", e)

    def register_callback(self, key, callbackfct):
        ''' register a callback function for a specific key - supports multiple callbacks per key '''
        if key not in self._callbackPerKey:
            self._callbackPerKey[key] = []
        if callbackfct not in self._callbackPerKey[key]:  # Avoid duplicate registrations
            self._callbackPerKey[key].append(callbackfct)
            print(f"Registered callback for key {key}. Total callbacks for this key: {len(self._callbackPerKey[key])}")
        

    def get_digitalin(self, digitalinid=1, timeout=1, is_blocking=True):
        """
        Get the current value of a digital input.
        
        Parameters:
        -----------
        digitalinid : int
            ID of the digital input (1, 2, or 3)
        timeout : float
            Timeout for the request in seconds
        is_blocking : bool
            Whether to wait for a response
            
        Returns:
        --------
        dict or None
            Response from the device containing digitalin status
        """
        path = '/digitalin_get'
        
        payload = {
            "task": path,
            "digitalinid": digitalinid
        }
        
        r = self._parent.post_json(path, payload, getReturn=is_blocking, timeout=timeout)
        return r

    def act_digitalin(self, timeout=1, is_blocking=False):
        """
        Trigger the digitalin act function.
        
        Parameters:
        -----------
        timeout : float
            Timeout for the request in seconds
        is_blocking : bool
            Whether to wait for a response
            
        Returns:
        --------
        dict or None
            Response from the device
        """
        path = '/digitalin_act'
        
        payload = {
            "task": path
        }
        
        r = self._parent.post_json(path, payload, getReturn=is_blocking, timeout=timeout)
        return r
