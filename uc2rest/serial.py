import serial
import serial.tools.list_ports
import time
import json


class Serial(object):
    
    def __init__(self, port, baudrate, timeout=1, parent=None):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self._parent = parent
        
        self.open() # creates self.serialdevice
        
    def open(self):
        '''Open the serial port'''

        self.is_connected = False
        serialdevice = None
        try:
            # most simple case: We know all parameters
            serialdevice = serial.Serial(port=self.serialport, baudrate=self.baudrate, timeout=1)
            self.is_connected = True
            time.sleep(2) # let it warm up
        except:
            # try to find the PORT
            _available_ports = serial.tools.list_ports.comports(include_links=False)
            for iport in _available_ports:
                # list of possible serial ports
                self._parent.logger.debug(iport.device)
                portslist = ("COM", "/dev/tt", "/dev/a", "/dev/cu.SLA","/dev/cu.wchusb", "/dev/cu.usbserial") # TODO: Hardcoded :/
                descriptionlist = ("CH340")
                if iport.device.startswith(portslist) or iport.description.find(descriptionlist) != -1:
                    try:
                        self.serialdevice = serial.Serial(port=iport.device, baudrate=self.baudrate, timeout=1)
                        self.is_connected = True # attempting to initiliaze connection
                        time.sleep(2)
                        correctFirmware = self.checkFirmware(serialdevice)
                        if correctFirmware:
                            self.serialport = iport.device
                            self._parent.logger.debug("We are connected: "+str(self.is_connected) + " on port: "+iport.device)
                            break
                    except Exception as e:
                        self._parent.logger.debug("Trying out port "+iport.device+" failed")
                        self._parent.logger.error(e)
                        self.is_connected = False
        
        return self.serialdevice                

    def checkFirmware(self, serialdevice, timeout=1):
        """Check if the firmware is correct"""
        path = "/state_get"
        _state = self.post_json(path, {"task":path}, isInit=True, timeout=timeout)
        if _state["identifier_name"] == "UC2_Feather": 
            return True
        else: return False


    def closeSerial(self):
        self.serialdevice.close()
        
    def reconnect(self):
        """Reconnect to serial device"""
        if self.is_serial:
            self.initSerial(self.serialport, self.baudrate)

    def get_json(self, path):
        """Perform an HTTP GET request and return the JSON response"""
        path = path.replace(self.base_uri,"")
        message = {"task":path}
        message = json.dumps(message)
        self.serialdevice.flushInput()
        self.serialdevice.flushOutput()
        returnmessage = self.serialdevice.write(message.encode(encoding='UTF-8'))
        return returnmessage

    def post_json(self, path, payload={}, headers=None, isInit=False, timeout=1):
        """Make an HTTP POST request and return the JSON response"""
        try:
            payload["task"]
        except:
            payload["task"] = path
        try:
            is_blocking = payload['isblock']
        except:
            is_blocking = True
        self.writeSerial(payload)
        
        returnmessage = self.readSerial(is_blocking=is_blocking, timeout=timeout)
        return returnmessage
        
    def writeSerial(self, payload):
        """Write JSON document to serial device"""
        try:
            self.serialdevice.flushInput()
            self.serialdevice.flushOutput()
        except Exception as e:
            self._parent.logger.error(e)
            try:
                del self.serialdevice
            except:
                pass
            self.is_connected=False
            # attempt to reconnect?
            try:
                self.open()
            except:
                return -1

        if type(payload)==dict:
            payload = json.dumps(payload)
        try:
            self.serialdevice.write(payload.encode(encoding='UTF-8'))
        except Exception as e:
            self._parent.logger.error(e)


    def readSerial(self, is_blocking=True, timeout = 15): # TODO: hardcoded timeout - not code
        """Receive and decode return message"""
        returnmessage = ''
        rmessage = ''
        _time0 = time.time()
        if is_blocking:
            while is_blocking:
                try:
                    rmessage =  self.serialdevice.readline().decode()
                    #self._parent.logger.debug(rmessage)
                    returnmessage += rmessage
                    if rmessage.find("--")==0:
                        break
                except:
                    pass
                if (time.time()-_time0)>timeout:
                    break
            # casting to dict
            try:
                returnmessage = json.loads(returnmessage.split("--")[0].split("++")[-1])
            except:
                self._parent.logger.debug("Casting json string from serial to Python dict failed")
                returnmessage = ""
        return returnmessage

        
class SerialDummy(object):
        
    def __init__(self, port, baudrate, timeout=1, parent=None):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self._parent = parent
        
        self.serialdevice = self.open()
        
    def open(self):
        '''Open the serial port'''
        return SerialDeviceDummy()                

    def checkFirmware(self, serialdevice):
        """Check if the firmware is correct"""
        return True
        
    def closeSerial(self):
        self.serialdevice.close()
        
    def reconnect(self):
        """Reconnect to serial device"""
        if self.is_serial:
            self.initSerial(self.serialport, self.baudrate)

    def get_json(self, path):
        """Perform an HTTP GET request and return the JSON response"""
        message = {"task":path}
        message = json.dumps(message)
        self.serialdevice.flushInput()
        self.serialdevice.flushOutput()
        returnmessage = self.serialdevice.write(message.encode(encoding='UTF-8'))
        return returnmessage

    def post_json(self, path, payload={}, headers=None, isInit=False, timeout=1):
        """Make an HTTP POST request and return the JSON response"""
        try:
            payload["task"]
        except:
            payload["task"] = path
        try:
            is_blocking = payload['isblock']
        except:
            is_blocking = True
        self.writeSerial(payload)
        #self._parent.logger.debug(payload)
        returnmessage = self.readSerial(is_blocking=is_blocking, timeout=timeout)
        return returnmessage
        
    def writeSerial(self, payload):
        """Write JSON document to serial device"""
        try:
            self.serialdevice.flushInput()
            self.serialdevice.flushOutput()
        except Exception as e:
            self._parent.logger.error(e)
            try:
                del self.serialdevice
            except:
                pass
            self.is_connected=False
            # attempt to reconnect?
            try:
                self.open()
            except:
                return -1

        if type(payload)==dict:
            payload = json.dumps(payload)
        try:
            self.serialdevice.write(payload.encode(encoding='UTF-8'))
        except Exception as e:
            self._parent.logger.error(e)


    def readSerial(self, is_blocking=True, timeout = 15): # TODO: hardcoded timeout - not code
        """Receive and decode return message"""
        returnmessage = ''
        rmessage = ''
        _time0 = time.time()
        if is_blocking:
            while is_blocking:
                try:
                    rmessage =  self.serialdevice.readline().decode()
                    #self._parent.logger.debug(rmessage)
                    returnmessage += rmessage
                    if rmessage.find("--")==0:
                        break
                except:
                    pass
                if (time.time()-_time0)>timeout:
                    break
            # casting to dict
            try:
                returnmessage = json.loads(returnmessage.split("--")[0].split("++")[-1])
            except:
                self._parent.logger.debug("Casting json string from serial to Python dict failed")
                returnmessage = ""
        return returnmessage

        
class SerialDeviceDummy(object):
        def __init__(self) -> None:
            pass
        
        def close(self):
            pass
        
        def flushInput(self):
            pass
        
        def flushOutput(self):
            pass
        
        def write(self, payload):
            pass
        
        def readline(self):
            return b'{"task":"dummy"}--'