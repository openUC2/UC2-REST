class SLM(object):
    
    
    def __init(self, parent):
        self._parent = parent
        
    '''
    ##############################################################################################################################
    SLM
    ##############################################################################################################################
    '''
    def send_SLM_circle(self, posX, posY, radius, color, timeout=1):
        '''
        Send an LED array pattern e.g. an RGB Matrix: led_pattern=np.zeros((3,8,8))
        '''
        path = '/slm_act'
        payload = {
            "posX": posX,
            "posY": posY,
            "radius": radius,
            "color": color,
            "slmMode": "circle"
        }
        r = self.post_json(path, payload, timeout=timeout)
        return r

    def send_SLM_clear(self, timeout=1):
        '''
        Send an LED array pattern e.g. an RGB Matrix: led_pattern=np.zeros((3,8,8))
        '''
        path = '/slm_act'
        payload = {
            "slmMode": "clear"
        }
        r = self.post_json(path, payload, timeout=timeout)
        return r

    def send_SLM_full(self, color, timeout=1):
        '''
        Send an LED array pattern e.g. an RGB Matrix: led_pattern=np.zeros((3,8,8))
        '''
        path = '/slm_act'
        payload = {
            "color":color,
            "slmMode": "full"
        }
        r = self.post_json(path, payload, timeout=timeout)
        return r


    def send_SLM_image(self, image, startX, startY, timeout=1):
        '''
        Send an LED array pattern e.g. an RGB Matrix: led_pattern=np.zeros((3,8,8))
        '''
        path = '/slm_act'

        endX = startX+image.shape[0]
        endY = startY+image.shape[1]

        payload = {
            "color": image[:].flatten().tolist(),
            "startX":startX,
            "startY":startY,
            "endX":endX,
            "endY":endY,
            "slmMode": "image"
        }
        r = self.post_json(path, payload, timeout=timeout)
        return r
