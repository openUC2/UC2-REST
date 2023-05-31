class Modules(object):
    ## Laser
    def __init__(self, parent):
        self._parent = parent
        # default settings
        self.default_modules = {
                "led":1,
                "motor":1,
                "slm":0,
                "home":1,
                "analogin":0,
                "pid":0,
                "laser":1,
                "dac":0,
                "analogout":0,
                "digitalout":0,
                "digitalin":1,
                "scanner":0,
                "wifi":0
            }
            
    def get_default_modules(self):
        return self.default_modules

    def get_modules(self, timeout=1):
        # returns a list of available modules that are set or not set in the ESP
        path = '/modules_get'
        try:
            r = self._parent.get_json(path)["modules"]
        except:
            r = None
        return r

    def set_modules(self, modules, timeout=1):
        path = '/modules_set'
        
        payload = {
            "task": path,
            "modules": modules
        }
        
        r = self._parent.post_json(path, payload)
        return r

