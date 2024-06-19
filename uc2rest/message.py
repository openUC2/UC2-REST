import numpy as np
import time
import json


gTIMEOUT = 1 # seconds to wait for a response from the ESP32
class Message(object):
    ''' 
    This class only parses incoming messages from the ESP32 that can be used e.g. for triggering events such as taking an image
    or converting hardware inputs such as button presses to software events
    '''
    def __init__(self, parent=None, nCallbacks = 10):
        self._parent = parent
        self.nCallbacks = nCallbacks
        # initialize the callback functions
        self.init_callback_functions(self.nCallbacks)
        
        # register a callback function for the motor status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_message, pattern="message")


    def _callback_message(self, data):
        ''' cast the json in the form:
        {
            "message":      {
                    "key":  1,
                    "data": 1
            }
        }       
        into seperated actions that follow this pattern:
        data lookup:
		{
			key 	|	 meaning 		|	value
			--------------------------------
			1		|	snap image		|	0
			2		|	exposure time	|	0....1000000
			3		|	gain			|	0...100
            ...
            
        Example:
            #%%
            import uc2rest
            import time
            
            port = "unknown"
            port = "/dev/cu.SLAB_USBtoUART"
            ESP32 = uc2rest.UC2Client(serialport=port, baudrate=500000, DEBUG=True)

            # register callback function for key/value pair 
            def my_callback_key1(value):
                print("Callback: ", value)
            ESP32.message.register_callback(1, my_callback_key1)

            while True:
                time.sleep(.1)
		*/
  
        '''
        try:
            # parse the message
            key = data["message"]["key"]
            value = data["message"]["data"]
            # trigger the action
            if self._callbackPerKey[key] is not None:
                self._callbackPerKey[key](value) # we call the function with the value
        except Exception as e:
            print("Error in _callback_message: ", e)

    def init_callback_functions(self, nCallbacks=10):
        ''' initialize the callback functions '''
        self._callbackPerKey = {}
        self.nCallbacks = nCallbacks
        for i in range(nCallbacks):
            self._callbackPerKey[i] = None
            
    def register_callback(self, key, callback):
        ''' register a callback function for a specific key '''
        self._callbackPerKey[key] = callback
        
    def trigger_message(self, key:int=1, value:int=1):
        # {"task": "/message_act", "message": 1, "key":1, "value":1}
        path = "/message_act"
        payload = {
            "task": path,
            "key": key,
            "value": value
            }
        r = self._parent.post_json(path, payload, getReturn=False)
        return r
