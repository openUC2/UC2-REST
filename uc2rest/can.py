class CAN(object):
    def __init__(self, parent):
        """
        CANController handles sending commands to a remote CAN device via the parent post_json interface.

        :param parent: Parent object with post_json(path, payload, getReturn, timeout, nResponses)
        """
        self._parent = parent
        
        # Store latest scan results
        self.scanResults = []
        self.deviceCount = 0
        
        # Register a callback function for the CAN status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_can_status, pattern="scan")
        
        # Announce a function that is called when we receive a CAN scan update through the callback
        self._callbackPerKey = {}
        self.nCallbacks = 10
        self._callbackPerKey = self.init_callback_functions(nCallbacks=self.nCallbacks)
        print(self._callbackPerKey)
    
    def init_callback_functions(self, nCallbacks=10):
        """Initialize the callback functions."""
        _callbackPerKey = {}
        self.nCallbacks = nCallbacks
        for i in range(nCallbacks):
            _callbackPerKey[i] = []
        return _callbackPerKey
    
    def _callback_can_status(self, data):
        """
        Cast the json in the form:
        {
            "scan": [
                {"canId": 20, "deviceType": 1, "status": 0, "deviceTypeStr": "laser", "statusStr": "idle"},
                {"canId": 10, "deviceType": 0, "status": 0, "deviceTypeStr": "motor", "statusStr": "idle"}
            ],
            "qid": 2,
            "count": 1
        }
        into the scan results array.
        """
        try:
            if "scan" in data:
                self.scanResults = data["scan"]
                self.deviceCount = data.get("count", len(self.scanResults))
                
                # Call registered callback function with scan results
                if callable(self._callbackPerKey[0]):
                    self._callbackPerKey[0](self.scanResults)
        except Exception as e:
            print("Error in _callback_can_status: ", e)
    
    def register_callback(self, key, callbackfct):
        """Register a callback function for a specific key."""
        self._callbackPerKey[key] = callbackfct

    def reboot_remote(self, qid=1, can_address=0, isBlocking=False, timeout=2):
        """
        Send a reboot signal to the remote CAN device.

        :param qid: Query ID for the CAN command (default: 1)
        :param isBlocking: If True, wait for response
        :param timeout: Timeout for the command in seconds
        :param can_address: Address of the CAN device to reboot (0 is master)
        :return: Response from the device
        """
        path = "/can_act"
        payload = {
            "task": path,
            "restart": int(can_address)
        }
        nResponses = 1 if isBlocking else 0
        # Send the payload to the parent, which handles the actual communication
        return self._parent.post_json(
            path,
            payload,
            getReturn=isBlocking,
            timeout=timeout if isBlocking else 0,
            nResponses=nResponses
        )

    def scan(self, qid=1, timeout=5):
        """
        Scan the CAN bus for connected devices.

        :param qid: Query ID for the CAN command (default: 1)
        :param timeout: Timeout for the scan in seconds (default: 5)
        :return: Response containing scan results with device information
                 Example: {
                     "scan": [
                         {"canId": 10, "deviceType": 0, "deviceTypeStr": "motor", "status": 0, "statusStr": "idle"},
                         {"canId": 20, "deviceType": 1, "deviceTypeStr": "laser", "status": 0, "statusStr": "idle"}
                     ],
                     "qid": 1,
                     "count": 2
                 }
        """
        path = "/can_act"
        payload = {
            "task": path,
            "scan": True,
            "qid": qid
        }
        return self._parent.post_json(
            path,
            payload,
            getReturn=True,
            timeout=timeout,
            nResponses=2
        )

    def get_available_devices(self, timeout=2):
        """
        Get list of available CAN devices.

        :param timeout: Timeout for the command in seconds (default: 2)
        :return: Response containing available CAN IDs and other bus information
                 Example: {
                     "input": {"task": "/can_get"},
                     "address": 1,
                     "addresspref": 1,
                     "addressgetcan": 1,
                     "nonworking": [0,0,0,...],
                     "available": [20,0,0,...],
                     "rx": 18,
                     "tx": 17
                 }
        """
        path = "/can_act"
        payload = {
            "task": path,
            "scan": True
        }
        return self._parent.post_json(
            path,
            payload,
            getReturn=True,
            timeout=timeout,
            nResponses=2
        )