class State(object):
    '''
    ##############################################################################################################################
    STATE
    ##############################################################################################################################
    '''

    def __init__(self, parent):
        self._parent = parent
        
    def get_state(self, timeout=3):
        path = "/state_get"

        r = self._parent.get_json(path, timeout=timeout)
        return r

    def delay(self, delay=1, getReturn=True):
        path = "/state_act"
        payload = {
            "task":path,
            "delay":delay
        }
        r = self._parent.post_json(path, payload, getReturn=getReturn)
        return r

    def set_state(self, debug=False, timeout=1):
        path = "/state_set"

        payload = {
            "task":path,
            "isdebug":int(debug)
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def isControllerMode(self, timeout=1):
        # returns True if PS controller is active
        path = "/state_get"
        payload = {
            "task":path,
            "pscontroller": 1
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try:
            return r["pscontroller"]
        except:
            return False
        
    def pairBT(self, timeout=1):
        path = "/bt_scan"
        payload={
            "dummy": 1}
        r = self._parent.post_json(path, payload, getReturn=False, timeout=timeout)
        return r
    
    def espRestart(self,timeout=1):
        # if isController =True=> only PS jjoystick will be accepted
        path = "/state_act"
        payload = {
            "restart":1
            }
        r = self._parent.post_json(path, payload, getReturn=False, timeout=timeout)
        return r

    def setControllerMode(self, isController=False, timeout=1):
        # if isController =True=> only PS jjoystick will be accepted
        path = "/state_act"
        payload = {
            "task":path,
            "pscontroller": isController
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def isBusy(self, timeout=1):
        path = "/state_get"
        payload = {
            "task":path,
            "isBusy": 1
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try:
            return r["isBusy"]
        except:
            return r


    def getHeap(self, timeout=1):
        path = "/state_get"
        payload = {
            "task":path,
            "heap": 1
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try:
            return r["heap"]
        except:
            return r