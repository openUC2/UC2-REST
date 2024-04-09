import time
import threading
class Temperature(object):

    def __init__(self, parent):
        self._parent = parent
        self._temperature = -273.15
        
        self.is_temperature_polling = False
        
        # register a callback function for the motor status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_temperature_status, pattern="heat")
        

    '''
    ##############################################################################################################################
    Temperature Controllers
    ##############################################################################################################################
    '''
    
    def start_temperature_polling(self, period=5):
        self.is_temperature_polling = True
        def _poll_temperature():
            while self.is_temperature_polling:
                path = "/heat_get"
                r = self._parent.get_json(path, timeout=0)
                time.sleep(period)
        threading.Thread(target=_poll_temperature).start()
        
    def stop_temperature_polling(self):
        self.is_temperature_polling = False
        
        
    def _callback_temperature_status(self, data):
        ''' cast the json in the form:
        {
        "qid":	0,
        "heat":	37.0
        }
        into the position array of the motors '''
        try:
            self._temperature = data["heat"]
        except Exception as e:
            print("Error in _callback_temperature_status: ", e)


    def stop_heating(self):
        r = self.set_temperature(active=0)
        return r

    def set_temperature(self, active=1, Kp=1000, Ki=0.1, Kd=0.1, target=37, timeout=600000000, updaterate=1000):
        # {"task": "/heat_act", "active":1, "Kp":1000, "Ki":0.1, "Kd":0.1, "target":37, "timeout":60000000, "updaterate":1000}
        path = "/heat_act"
        payload = {
            "task": path,
            "active": int(active),
            "Kp": Kp,
            "Ki": Ki,
            "Kd": Kd,
            "target": target,
            "timeout": timeout,
            "updaterate": updaterate
            }
        r = self._parent.post_json(path, payload, getReturn=False)
        return r

    def get_temperature(self, timeout=0.5, isBlocking=False):
        # {"task": "/heat_get"}
        if isBlocking:
            path = "/heat_get"
            r = self._parent.get_json(path, timeout=timeout)
            try:
                r = r["heat"]
            except:
                r = -273.15
            return r
        else:
            return self._temperature

if __name__ == "__main__":

    #%%
    import uc2rest
    import time
    port = "/dev/cu.SLAB_USBtoUART"
    ESP32 = uc2rest.UC2Client(serialport=port, DEBUG=True)
    ESP32.temperature.set_temperature(active=1, Kp=1000, Ki=0.1, Kd=0.1, target=37, timeout=600000, updaterate=1000)
    for i in range(10):
        print(ESP32.temperature.get_temperature())
        time.sleep(1)
    ESP32.temperature.stop_heating()
    ESP32.close()
