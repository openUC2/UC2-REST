class Temperature(object):

    def __init__(self, parent):
        self._parent = parent

    '''
    ##############################################################################################################################
    Temperature Controllers
    ##############################################################################################################################
    '''

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

    def get_temperature(self, timeout=0.5):
        path = "/heat_get"
        r = self._parent.get_json(path, timeout=timeout)
        try:
            r = r["heat"]
        except:
            r = -273.15
        return r


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
