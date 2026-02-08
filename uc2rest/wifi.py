    
class Wifi(object):
    
    def __init__(self, parent):
        self._parent = parent
        
    def isConnected(self):
        '''
        deprecated 
        '''
        pass 
    
    def get_json(self, path):
        '''
        deprecated 
        '''
        pass 

    def post_json(self, path, payload={}, headers=None, isInit=False, timeout=1):
        '''
        deprecated 
        '''
        pass 
 
    @property
    def base_uri(self):
        return f"http://{self.host}:{self.port}"


    def send_jpeg(self, image):
        '''
        deprecated 
        '''
        pass 

    def scanWifi(self, timeout=3):
        path = '/wifi/scan'
        payload={
            "task": path
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

        