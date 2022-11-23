class Analog(object):

    def __init__(self, parent):
        self._parent = parent
        
    '''
    ##############################################################################################################################
    Sensors
    ##############################################################################################################################
    '''
    def read_sensor(self, sensorID=0, NAvg=100):
        path = "/readanalogin_act"
        payload = {
            "readanaloginID": sensorID,
            "nanaloginavg": NAvg,
        }
        r = self._parent.post_json(path, payload)
        try:
            sensorValue = r['analoginVAL']
        except:
            sensorValue = None
        return sensorValue
    # TODO: Get/SET methods missing
    '''
    (*WifiController::getJDoc())["analoginVAL"] = returnValue;
    (*WifiController::getJDoc())["readanaloginPIN"] = analoginpin;
    (*WifiController::getJDoc())["N_analogin_avg"] = N_analogin_avg;
    (*WifiController::getJDoc())["readanaloginID"] = readanaloginID;
    (*WifiController::getJDoc())["N_analogin_avg"] = N_analogin_avg;
    '''

    def get_analog(self, readanaloginID=1):
        # readanaloginID 1,2,3
        path = "/readanalogin_get"
        payload = {
            "task": path,
            "readanaloginID": readanaloginID, 
            }
        r = self._parent.post_json(path, payload)
        return r
    
    def set_analog(self, readanaloginID=1, readanaloginPIN=15, nanaloginavg=1):
        # readanaloginID 1,2,3
        path = "/readanalogin_set"
        payload = {
            "task": path,
            "readanaloginID": readanaloginID, 
            "readanaloginPIN": readanaloginPIN, 
            "nanaloginavg": nanaloginavg
            }
        r = self._parent.post_json(path, payload)
        return r

