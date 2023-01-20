class DigitalOut(object):

    def __init__(self, parent):
        self._parent = parent
        self._logger = parent.logger
    
    
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
