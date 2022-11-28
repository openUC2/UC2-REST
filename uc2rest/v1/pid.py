class PID(object):
    
    def __init__(self, parent):
        self._parent = parent
        
    '''
    ##############################################################################################################################
    PID controllers
    ##############################################################################################################################
    '''
    def set_pidcontroller(self, PIDactive=1, Kp=100, Ki=10, Kd=1, target=500, PID_updaterate=200):
        #{"task": "/PID_act", "PIDactive":1, "Kp":100, "Ki":10, "Kd":1, "target": 500, "PID_updaterate":200}
        #TOOD: PUt this into a class structure
        path = "/PID_act"
        payload = {
            "task": path,
            "PIDactive": PIDactive,
            "Kp": Kp,
            "Ki": Ki,
            "Kd": Kd,
            "target": target,
            "PID_updaterate": PID_updaterate
            }
        r = self.post_json(path, payload)
        return r
