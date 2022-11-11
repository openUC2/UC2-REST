    
class Wifi(object):
    
    def __init__(self, parent):
        self._parent = parent
        
    def isConnected(self):
        # check if client is connected to the same network
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((self.host, int(self.port)))
            s.settimeout(2)
            s.shutdown(2)
            return True
        except:
            return False

    
    def get_json(self, path):
        if self.is_connected and self.is_wifi:
            if not path.startswith("http"):
                path = self.base_uri + path
            try:
                r = requests.get(path)
                r.raise_for_status()
                self.is_connected = True

                self.getmessage = r.json()
                self.is_sending = False
                return self.getmessage

            except Exception as e:
                if IS_IMSWITCH: self.__logger.error(e)
                self.is_connected = False
                self.is_sending = False
                # not connected
                return None

    def post_json(self, path, payload={}, headers=None, isInit=False, timeout=1):
        """Make an HTTP POST request and return the JSON response"""
        if self.is_connected and self.is_wifi:
            if not path.startswith("http"):
                path = self.base_uri + path
            if headers is None:
                headers = self.headers
            try:
                r = requests.post(path, json=payload, headers=headers,timeout=timeout)
                r.raise_for_status()
                r = r.json()
                self.is_connected = True
                self.is_sending = False
                return r
            except Exception as e:
                if IS_IMSWITCH: self.__logger.error(e)
                self.is_connected = False
                self.is_sending = False
                # not connected
                return None

 
    @property
    def base_uri(self):
        return f"http://{self.host}:{self.port}"


    def send_jpeg(self, image):
        if is_cv2:
            temp = NamedTemporaryFile()

            #add JPEG format to the NamedTemporaryFile
            iName = "".join([str(temp.name),".jpg"])

            #save the numpy array image onto the NamedTemporaryFile
            cv2.imwrite(iName,image)
            _, img_encoded = cv2.imencode('test.jpg', image)

            content_type = 'image/jpeg'
            headers = {'content-type': content_type}
            payload = img_encoded.tostring()
            path = '/uploadimage'

            #r = self.post_json(path, payload=payload, headers = headers)
            #requests.post(self.base_uri + path, data=img_encoded.tostring(), headers=headers)
            files = {'media': open(iName, 'rb')}
            if self.is_connected:
                requests.post(self.base_uri + path, files=files)



    def scanWifi(self, timeout=3):
        path = '/wifi/scan'
        payload={
            "task": path
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

        