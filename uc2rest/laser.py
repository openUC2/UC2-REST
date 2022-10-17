class Laser(object):
    
    def __init__(self, parent):
        self._parent = parent
        
        
    def set_laser(self, channel=1, value=0, auto_filterswitch=False,
                        filter_axis=-1, filter_position = None,
                        despeckleAmplitude = 0.,
                        despecklePeriod=10, timeout=20, is_blocking = True):
        if channel not in (0,1,2,3):
            if channel=="R":
                channel = 1
            elif channel=="G":
                channel = 2
            elif channel=="B":
                channel = 3

        if auto_filterswitch and value >0:
            if filter_position is None:
                if channel==1:
                    filter_position_toGo = self.filter_pos_1
                if channel==2:
                    filter_position_toGo = self.filter_pos_2
                if channel==3:
                    filter_position_toGo = self.filter_pos_3
                if channel=="LED":
                    filter_position_toGo = self.filter_pos_LED
            else:
                filter_position_toGo = filter_position

            self.switch_filter(filter_pos=filter_position_toGo, filter_axis=filter_axis, timeout=timeout,is_blocking=is_blocking)

        path = '/laser_act'

        payload = {
            "task": path,
            "LASERid": channel,
            "LASERval": value,
            "LASERdespeckle": int(value*despeckleAmplitude),
            "LASERdespecklePeriod": int(despecklePeriod),

        }
        self._parent.logger.debug("Setting Laser "+str(channel)+", value: "+str(value))
        r = self._parent.post_json(path, payload)
        return r
