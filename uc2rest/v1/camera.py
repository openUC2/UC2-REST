from PIL import Image
import base64
import io
import numpy as np

class Camera(object):
    
    def __init__(self, parent, width=320, height=240, fps=10):
        self._parent = parent
        self.width = width
        self.height = height
        self.lastFrame = 255*np.random.randn(self.height, self.width)
        
        
    def set_camera(self, width=None, height=None, fps=None, timeout=3):
        path = "/camera_set"
        payload = {"path":path,
                   "width":width,
                   "height":height,
                   "fps":fps}
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r
    
    def get_camera(self):
        path = "/camera_get"
        return None
    
    def get_frame(self, timeout=5):
        path = "/camera_act"
        payload = {
            "grabimage":1
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        
        #%%

        try:
            #read image and decode
            imageB64 = r['frame']
            image = np.array(Image.open(io.BytesIO(base64.b64decode(imageB64))))

        except Exception as e:
            self._parent.logger.error(f"Error: {e}")
            image = None
            image = self.lastFrame
            
        return image
            

    def set_led(self, value):
        path = "/led_act"
        payload = {
            "value":value
        }
        r = self._parent.post_json(path, payload, timeout=2)
        
        return r
