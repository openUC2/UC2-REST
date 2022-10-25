class Analog(object):

    def __init__(self, parent):
        self._parent = parent
        
    '''
    ##############################################################################################################################
    Sensors
    ##############################################################################################################################
    '''
    def read_sensor(self, sensorID=0, NAvg=100):
        path = "/readsensor_act"
        payload = {
            "readsensorID": sensorID,
            "N_sensor_avg": NAvg,
        }
        r = self.post_json(path, payload)
        try:
            sensorValue = r['sensorValue']
        except:
            sensorValue = None
        return sensorValue
    # TODO: Get/SET methods missing