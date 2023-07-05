try:
    import serial
    import serial.tools.list_ports
except:
    print("No serial available - running on pyscript?")
import time
import json

T_SERIAL_WARMUP = 3 # the time to wait for the serial to warm up

class Serial(object):

    def __init__(self, port, baudrate, timeout=1, identity="UC2_Feather", parent=None, DEBUG=False):
        self.serialport = port
        self.baudrate = baudrate
        self.timeout = timeout
        self._parent = parent
        self.identity = identity
        self.DEBUG = DEBUG

        self.isSafetyBreak = False

        # default version (v1 or v2) for the API
        self.versionFirmware = "V2.0"

        self.NumberRetryReconnect = 0
        self.MaxNumberRetryReconnect = 0

        self.open() # creates self.serialdevice

        # switch between different api versions
        if self.versionFirmware == "V1.2":
            self._parent.APIVersion = 1
        elif self.versionFirmware == "V2.0":
            self._parent.APIVersion = 2


    def open(self):
        '''Open the serial port'''
        self.is_connected = False
        self.serialdevice = None
        try:
            # most simple case: We know all parameters
            self.serialdevice = serial.Serial(port=self.serialport, baudrate=self.baudrate, timeout=1)
            self.is_connected = True
            time.sleep(T_SERIAL_WARMUP) # let it warm up
            correctFirmware = self.checkFirmware()
            if not correctFirmware:
                raise Exception("Wrong firmware")

            self._parent.logger.debug("We are connected: "+str(self.is_connected) + " on port: "+self.serialdevice.port)
            return self.serialdevice
        except Exception as e:
            # try to find the PORT
            self._parent.logger.error(e)
            _available_ports = serial.tools.list_ports.comports(include_links=False)

            portslist = ("COM", "/dev/tt", "/dev/a", "/dev/cu.SLA","/dev/cu.wchusb")#, "/dev/cu.usbserial") # TODO: Hardcoded :/
            descriptionlist = ("CH340", "CP2102")
            for iport in _available_ports:
                # list of possible serial ports
                self._parent.logger.debug(iport.device)
                if iport.device.startswith(portslist) or iport.description.startswith(descriptionlist):
                    try:
                        self.serialdevice = serial.Serial(port=iport.device, baudrate=self.baudrate, timeout=1)
                        self.serialdevice.write_timeout=1
                        self.is_connected = True # attempting to initiliaze connection
                        time.sleep(T_SERIAL_WARMUP) # let it warm up and wait until debugging messages may vanish
                        correctFirmware = self.checkFirmware()
                        if correctFirmware:
                            self.serialport = iport.device
                            self._parent.logger.debug("We are connected: "+str(self.is_connected) + " on port: "+self.serialdevice.port)
                            self.NumberRetryReconnect=0
                            return self.serialdevice
                    except Exception as e:
                        self._parent.logger.debug("Trying out port "+iport.device+" failed")
                        self._parent.logger.error(e)
                        self.is_connected = False
        # last resort: we are not connected
        self.serialport = "NotConnected"
        self.serialdevice = SerialDeviceDummy()
        self._parent.logger.debug("No USB device connected! Using DUMMY!")
        return self.serialdevice

    def checkFirmware(self, timeout=1):
        """Check if the firmware is correct"""
        path = "/state_get"
        _state = self.post_json(path, {"task":path}, timeout=timeout)
        try:
            self.versionFirmware = _state["identifier_id"]
        except Exception as e:
            self._parent.logger.error(e)
            self.versionFirmware = "V2.0"

        if _state["identifier_name"] == self.identity:
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
        message = {"task":path}
        message = json.dumps(message)
        self.serialdevice.flushInput()
        self.serialdevice.flushOutput()
        self._parent.logger.debug(message)
        returnmessage = self.serialdevice.write(message.encode(encoding='UTF-8'))
        return returnmessage

    def post_json(self, path, payload={}, getReturn=True, timeout=1):
        """Make an HTTP POST request and return the JSON response"""
        self.is_sending = True
        if "task" not in payload:
            payload["task"] = path

        if "isblock" in payload:
            is_blocking = payload["isblock"]
        else:
            is_blocking = True

        # write message to the serial
        self.writeSerial(payload)

        if getReturn:
            # we read the return message
            #self._parent.logger.debug(payload)
            returnmessage = self.readSerial(is_blocking=is_blocking, timeout=timeout)
            if returnmessage == 'deserializeJson() failed: NoMemory':
                # TODO: We will loose values here - need to set xyz coordinates!!
                self.serialdevice.close()
                self._parent.state.espRestart()
                self._parent.state.restart()
                self.open()
        else:
            returnmessage = False
        self.is_sending = False
        return returnmessage

    def writeSerial(self, payload):
        """Write JSON document to serial device"""
        try:
            if self.serialport == "NotConnected" and self.NumberRetryReconnect<self.MaxNumberRetryReconnect:
                # try to reconnect
                self._parent.logger.debug("Trying to reconnect to serial device: "+str(self.NumberRetryReconnect))
                self.NumberRetryReconnect += 1
                self.open()

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
            if self.DEBUG: self._parent.logger.debug(payload)
            self.serialdevice.write(payload.encode(encoding='UTF-8'))
        except Exception as e:
            self._parent.logger.error(e)

    def breakCurrentCommunication(self):
        # this breaks the wait-for-return command in readserial
        self.isSafetyBreak = True
        self.serialdevice.flushInput()
        self.serialdevice.flushOutput()


    def readSerial(self, is_blocking=True, timeout = 1): # TODO: hardcoded timeout - not code
        """Receive and decode return message"""
        returnmessage = ''
        _returnmessage = ''
        rmessage = ''
        _time0 = time.time()
        if is_blocking:
            while is_blocking and not self.isSafetyBreak and not self.serialport=="NotConnected":
                try:
                    rmessage =  self.serialdevice.readline().decode()
                    if self.DEBUG: self._parent.logger.debug(rmessage)
                    returnmessage += rmessage
                    if rmessage.find("--")==0:
                        break
                except:
                    pass
                if (time.time()-_time0)>timeout:
                    break
            # casting to dict
            try:
                # TODO: check if this is a valid JSON
                _returnmessage = returnmessage.split("\n--")[0].split("\n++")[-1].replace("\r","").replace("\n", "").replace("'", '"')
                if self.DEBUG: self._parent.logger.debug(returnmessage)
                _returnmessage = json.loads(_returnmessage)
            except Exception as e:
                if self.DEBUG: self._parent.logger.debug("Casting json string from serial to Python dict failed")
            self.isSafetyBreak = False
        return _returnmessage

class SerialManagerWrapper(object):

    def __init__(self, SerialManager, DEBUG = True, parent=None) -> None:
        self._parent=parent
        self._parent.logger.debug("SerialManagerWrapper init")
        self.SerialManager = SerialManager
        self.isSafetyBreak = False
        self.DEBUG = DEBUG

    async def post_json(self, path, payload={}, getReturn=True, timeout=1):
        if "task" not in payload:
            payload["task"] = path

        if "isblock" in payload:
            is_blocking = payload["isblock"]
        else:
            is_blocking = True

        # write message to the serial
        await self.writeSerial(payload)
        return ''
        print(3)
        if getReturn:
            # we read the return message
            #self._parent.logger.debug(payload)
            returnmessage = self.readSerial(is_blocking=is_blocking, timeout=timeout)
        else:
            returnmessage = False
        return returnmessage

    async def writeSerial(self, payload):
        """Write JSON document to serial device"""
        if type(payload)==dict:
            payload = json.dumps(payload)
        try:
            if self.DEBUG: self._parent.logger.debug(payload)
            await self.SerialManager.write(payload)
        except Exception as e:
            self._parent.logger.error(e)

    async def readSerial(self, is_blocking=True, timeout = 1): # TODO: hardcoded timeout - not code
        """Receive and decode return message"""
        returnmessage = ''
        _returnmessage = ''
        rmessage = ''
        _time0 = time.time()
        print("Reading serial")
        if is_blocking:
            while is_blocking and not self.isSafetyBreak:
                try:
                    rmessage = await self.SerialManager.read().decode()
                    if self.DEBUG: self._parent.logger.debug(rmessage)
                    returnmessage += rmessage
                    if rmessage.find("--")==0:
                        break
                except:
                    pass
                if (time.time()-_time0)>timeout:
                    break
            # casting to dict
            try:
                # TODO: check if this is a valid JSON
                _returnmessage = returnmessage.split("\n--")[0].split("\n++")[-1].replace("\r","").replace("\n", "").replace("'", '"')
                if self.DEBUG: self._parent.logger.debug(returnmessage)
                _returnmessage = json.loads(_returnmessage)
            except Exception as e:
                if self.DEBUG: self._parent.logger.debug("Casting json string from serial to Python dict failed")
            self.isSafetyBreak = False
        return _returnmessage
class SerialDummy(object):

    def __init__(self, port, baudrate, timeout=1, parent=None):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self._parent = parent
        self.versionFirmware = "Dummy"
        self.is_sending = False
        self.serialdevice = self.open()

    def open(self):
        '''Open the serial port'''
        return SerialDeviceDummy()

    def checkFirmware(self):
        """Check if the firmware is correct"""
        return True

    def close(self):
        self.closeSerial()

    def closeSerial(self):
        self.serialdevice.close()

    def reconnect(self):
        """Reconnect to serial device"""
        if self.is_serial:
            self.initSerial(self.serialport, self.baudrate)

    def get_json(self, path):
        """Perform an HTTP GET request and return the JSON response"""
        self.is_sending = True
        message = {"task":path}
        message = json.dumps(message)
        self.serialdevice.flushInput()
        self.serialdevice.flushOutput()
        returnmessage = self.serialdevice.write(message.encode(encoding='UTF-8'))
        self.is_sending = False
        return returnmessage

    def post_json(self, path, payload={}, getReturn=True, timeout=1):
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
        if getReturn:
            # we read the return message
            #self._parent.logger.debug(payload)
            returnmessage = self.readSerial(is_blocking=False, timeout=timeout)
        else:
            returnmessage = None
        return returnmessage

    def writeSerial(self, payload):
        """Write JSON document to serial device"""
        try:
            # clear any data that's in the buffer
            self.serialdevice.readline()
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
