import numpy as np

gTimeout = 2

class LedMatrix(object):
    def __init__(self, parent, NLeds=64):
        """
        This class handles sending JSON commands for controlling a NeoPixel or LED matrix device.
        The 'parent' is assumed to be an object providing post_json(...) and get_json(...) methods.
        """
        self.NLeds = NLeds
        self.Nx = self.Ny = int(np.sqrt(NLeds)) if NLeds > 0 else 8

        # We assume the pattern is binary (0 or 1) for an NxN grid, stored in ledpattern NxN or flattened.
        # We store an RGB for each LED if needed. -1 indicates "unset" or no color assigned.
        self.ledpattern = np.ones((self.NLeds, 3)) * -1

        self._parent = parent  # must have post_json() etc.
        self.timeout = 1
        self.intensity = (255, 255, 255)

        # Maps textual modes to an integer, used for backward compatibility
        self.ledArrayModes = {
            "array": 0,
            "full": 1,
            "single": 2,
            "off": 3,
            "left": 4,
            "right": 5,
            "top": 6,
            "bottom": 7,
            "multi": 8
        }

        self.currentLedArrayMode = "full"

    ######################################################################################################
    # BASE / LEGACY METHODS
    ######################################################################################################

    def send_LEDMatrix_array(self, led_pattern, getReturn=True, timeout=gTimeout):
        """
        Send an LED array pattern e.g. a list of { id, r, g, b } or a NumPy array shape (N,3).
        Produces JSON:
        {
          "task": "/ledarr_act",
          "led": {
            "action": "array",
            "LEDArrMode": 0,
            "led_array": [
               {"id":0,"r":...,"g":...,"b":...},
               ...
            ]
          }
        }
        """
        path = "/ledarr_act"

        # If led_pattern is a NumPy array, convert it to a list of dict
        if not isinstance(led_pattern, list):
            if len(led_pattern.shape) == 3:
                # flatten 3D into 2D
                led_pattern = np.reshape(led_pattern, (led_pattern.shape[0]*led_pattern.shape[1], led_pattern.shape[2]))

            pattern_list = []
            for i in range(led_pattern.shape[0]):
                pattern_list.append({
                    "id": i,
                    "r": int(led_pattern[i, 0]),
                    "g": int(led_pattern[i, 1]),
                    "b": int(led_pattern[i, 2])
                })
        else:
            pattern_list = led_pattern

        payload = {
            "task": path,
            "qid": 0,  # or fill in dynamically if you like
            "led": {
                "action": "array",
                "LEDArrMode": self.ledArrayModes["array"],
                "led_array": pattern_list
            }
        }

        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        if not getReturn or timeout == 0:
            r = {"success": 1}
        self.currentLedArrayMode = "array"
        return r

    def send_LEDMatrix_full(self, intensity=(255, 255, 255), getReturn=True, timeout=gTimeout):
        """
        Fill all LEDs with the same (r,g,b). 
        Creates JSON with: "action": "fill"
        """
        path = "/ledarr_act"
        payload = {
            "task": path,
            "qid": 0,
            "led": {
                "action": "fill",
                "LEDArrMode": self.ledArrayModes["full"],
                "r": int(intensity[0]),
                "g": int(intensity[1]),
                "b": int(intensity[2])
            }
        }

        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        if not getReturn or timeout == 0:
            r = {"success": 1}
        self.currentLedArrayMode = "full"
        return r

    def send_LEDMatrix_single(self, indexled=0, intensity=(255, 255, 255), getReturn=True, timeout=gTimeout):
        """
        Update only a single LED with color (r,g,b).
        Creates JSON: "action":"single"
        """
        path = "/ledarr_act"
        payload = {
            "task": path,
            "qid": 0,
            "led": {
                "action": "single",
                "LEDArrMode": self.ledArrayModes["single"],
                "ledIndex": int(indexled),
                "r": int(intensity[0]),
                "g": int(intensity[1]),
                "b": int(intensity[2])
            }
        }

        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        if not getReturn or timeout == 0:
            r = {"success": 1}
        self.currentLedArrayMode = "single"
        return r

    def get_LEDMatrix(self, timeout=gTimeout):
        """
        Request info from the device about pin/LED count, etc.
        Usually the device responds with JSON containing these details.
        """
        path = "/ledarr_get"
        payload = {"task": path}
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        return r

    def setSingle(self, indexled, state):
        """
        High-level usage: set a single LED's "state" (0 or 1), then send it with intensity scaling.
        """
        ix = indexled // self.Nx
        iy = indexled % self.Nx
        self.ledpattern[indexled] = (state, state, state)
        # If your wiring is zig-zag or reversed, you may need to adapt index
        if ix % 2 != 0:
            indexled = (ix * self.Nx) + (self.Ny - iy - 1)

        intensityScaled = np.array(state) * np.array(self.intensity)
        self.send_LEDMatrix_single(indexled=indexled, intensity=intensityScaled, timeout=self.timeout)
        return self.ledpattern

    def setAll(self, state: int, intensity:int =None, getReturn=True):
        """
        Turn on or off all LEDs at a certain intensity. 
        If state is a boolean array or a single boolean, we set them all.
        """
        onval = np.sum(state) > 0
        if intensity is not None:
            self.intensity = intensity
        intensity2display = np.array(self.intensity) * onval
        self.send_LEDMatrix_full(intensity=intensity2display, getReturn=getReturn)
        self.ledpattern = onval * np.ones((self.NLeds, 3))
        return self.ledpattern

    def setIntensity(self, intensity:int, getReturn:bool=True):
        self.intensity = intensity
        self.setPattern(getReturn=getReturn)

    def setPattern(self, getReturn:bool=True, ledpattern=None):
        """
        If ledpattern is provided, store it, then send to device.
        If the entire pattern is "on", we do a 'full' fill for speed,
        otherwise we do 'array' updates for each LED.
        """
        if ledpattern is not None:
            self.ledpattern = ledpattern
        pattern2send = (self.ledpattern >= 1) * self.intensity

        if np.sum(self.ledpattern, 0)[0] == self.ledpattern.shape[0]:
            # All on => just do a fill
            self.send_LEDMatrix_full(pattern2send[0, :], getReturn=getReturn)
        else:
            # partial => send an array of per-LED
            self.send_LEDMatrix_array(pattern2send, getReturn=getReturn)
        return self.ledpattern

    def getPattern(self):
        return self.ledpattern

    def set_led(self, colour=(0, 0, 0)):
        """
        (Legacy) Possibly sets a single color. 
        Demo only - not used in the new command style.
        """
        path = "/led"
        payload = {"r": colour[0], "g": colour[1], "b": colour[2]}
        r = self._parent.post_json(path, payload)
        return r

    def get_ledpin(self):
        """
        Another legacy method that might do a GET request.
        """
        path = "/ledarr_get"
        r = self._parent.get_json(path, getReturn=True, timeout=1)
        return r

    ######################################################################################################
    # NEW METHODS FOR "OFF", "RINGS", "CIRCLES", "HALVES", etc.
    ######################################################################################################
    def send_LEDMatrix_off(self, getReturn=True, timeout=gTimeout):
        """
        Turn all LEDs off.
        JSON: "action":"off"
        """
        path = "/ledarr_act"
        payload = {
            "task": path,
            "qid": 0,
            "led": {
                "action": "off"
            }
        }
        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        return r

    def send_LEDMatrix_halves(self, region="left", intensity=(255,255,255), getReturn=True, timeout=gTimeout):
        """
        Light only "left"/"right"/"top"/"bottom" half in color, rest is dark.
        JSON: "action":"halves", "region": <string>
        """
        if type(intensity)==int:
            intensity = (intensity, intensity, intensity)        
        path = "/ledarr_act"
        payload = {
            "task": path,
            "qid": 0,
            "led": {
                "action": "halves",
                "region": region,
                "r": int(intensity[0]),
                "g": int(intensity[1]),
                "b": int(intensity[2])
            }
        }
        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        return r

    def send_LEDMatrix_rings(self, radius=4, intensity=(255,255,255), getReturn=True, timeout=gTimeout):
        """
        Draw a ring of radius N. Often done by filling the circle and carving out center to make it hollow.
        JSON: "action":"rings", "radius": <int>
        """
        if type(intensity)==int:
            intensity = (intensity, intensity, intensity)
        path = "/ledarr_act"
        payload = {
            "task": path,
            "qid": 0,
            "led": {
                "action": "rings",
                "radius": radius,
                "r": int(intensity[0]),
                "g": int(intensity[1]),
                "b": int(intensity[2])
            }
        }
        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        return r

    def send_LEDMatrix_circles(self, radius=4, intensity=(255,255,255), getReturn=True, timeout=gTimeout):
        """
        Draw a filled circle of radius N in color.
        JSON: "action":"circles", "radius": <int>
        """
        if type(intensity)==int:
            intensity = (intensity, intensity, intensity)
        path = "/ledarr_act"
        payload = {
            "task": path,
            "qid": 0,
            "led": {
                "action": "circles",
                "radius": radius,
                "r": int(intensity[0]),
                "g": int(intensity[1]),
                "b": int(intensity[2])
            }
        }
        r = self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)
        return r

    def send_LEDMatrix_status(self, status="idle"):
        """
        Set the status of the LED matrix to "idle" or "busy".
        JSON: "action":"status", "status": <string>
        """
        if status not in ["error", "idle", "warn", "success", "busy", "rainbow"]:
            status = "idle"
        path = "/ledarr_act"
        payload = {
            "task": path,
            "qid": 0,
            "led": {
                "action": "status",
                "status": status
            }
        }
        r = self._parent.post_json(path, payload)
        return r