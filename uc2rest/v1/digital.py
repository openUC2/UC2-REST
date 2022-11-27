class digital(object):

    def __init__(self, parent):
        self._parent = parent
        self._logger = parent._logger
    
    def sendTrigger(self, triggerId=0):
        path = '/digital_act'

        payload = {
            "task": path,
            "digitalid": triggerId,
            "digitalval": -1,
        }

        r = self.post_json(path, payload)
        return r
