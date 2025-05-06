class LCDDisplay:
    def __init__(self, parent):
        """
        LCD controller interface. 
        Sends JSON commands to draw on the LCD via the parent object.
        """
        self._parent = parent
        self.path = "/lcd_act"
        self.timeout = 10
        self.color = (255, 255, 255)  # Default color

    def _send(self, action, isBlocking=True, **kwargs):
        payload = {
            "task": self.path,
            "action": action,
            **kwargs
        }
        return self._parent.post_json(self.path, payload, getReturn=isBlocking, nResponses=1, timeout=self.timeout)

    def set_grating(self, perdiod, r=255, g=255, b=255, horizontal=True):
        # {"task":"/lcd_act", "action":"gratingh","period":3,"r":255,"g":255,"b":255}
        if horizontal:
            return self._send("gratingh", period=perdiod, r=r, g=g, b=b)
        else:
            return self._send("gratingv", period=perdiod, r=r, g=g, b=b)
        
    def set_timeout(self, timeout=5):
        """
        Set the timeout for the LCD display.
        :param timeout: Timeout in seconds.
        """
        self.timeout = timeout
                
    def set_color(self, r=255, g=255, b=255):
        self.color = (r, g, b)

    def clear(self, r=0, g=0, b=0):
        return self._send("clear", r=r, g=g, b=b)

    def hline(self, x, y, length, width=1):
        r, g, b = self.color
        # {"task":"/lcd_act","action":"hline","x":0,"y":10,"len":800,"width":1,"r":255,"g":255,"b":0}
        return self._send("hline", x=x, y=y, len=length, width=width, r=r, g=g, b=b)

    def vline(self, x, y, length, width=1):
        r, g, b = self.color
        return self._send("vline", x=x, y=y, len=length, width=width, r=r, g=g, b=b)

    def point(self, x, y, diam=1):
        r, g, b = self.color
        return self._send("point", x=x, y=y, diam=diam, r=r, g=g, b=b)

    def rect(self, x, y, len, width):
        r, g, b = self.color
        return self._send("rect", x=x, y=y, len=len, width=width, r=r, g=g, b=b)

    def fill(self, x, y, len, width):
        r, g, b = self.color
        return self._send("fill", x=x, y=y, len=len, width=width, r=r, g=g, b=b)
