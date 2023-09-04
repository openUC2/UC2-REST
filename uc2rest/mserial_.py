import serial
import json
import queue
import threading
import time

T_SERIAL_WARMUP = 3
class Serial:
    def __init__(self, port, baudrate=115200, timeout=5, 
                 identity="UC2_Feather", parent=None, DEBUG=False):
        
        self.ser = None
        self.serialport = port
        self.baudrate = baudrate
        self.timeout = timeout
        self._parent = parent
        self.identity = identity
        self.DEBUG = DEBUG
        self.is_connected = False

        # setup command queue        
        self.resetLastCommand = False
        self.responses = {}
        self.commands = {}
        self.lock = threading.Lock()
        self.running = True
        self.identifier_counter = 0 # Counter for generating unique identifiers
        self.thread = threading.Thread(target=self._process_commands)
        self.thread.start()
        self.serial_lock = threading.Lock()
        
        self.currentIdentifier = -1

        # initialize serial connection        
        self.ser = self.openDevice(port, baudrate, timeout)
        
    def setCurrentIdentifier(self, identifier):
        self.currentIdentifier = identifier
    
    def getCurrentIdentfier(self):
        return self.currentIdentifier
    
    def breakCurrentCommunication(self):
        self.resetLastCommand = True

    def _freeSerialBuffer(self, ser, timeout=5):
        t0 = time.time()
        # free up any old data
        while True:
            try:
                readLine = ser.readline().decode('utf-8').strip()
                if readLine == "":
                    break
            except:
                pass
            if time.time()-t0 > timeout:
                return
            
    def openDevice(self, port, baud_rate, timeout=5):
        try: 
            ser = serial.Serial(port, baud_rate, timeout=1)
            ser.write_timeout = 1
        except:
            ser = self.findCorrectSerialDevice()
            
        self.is_connected = True
        
        # TODO: Need to be able to auto-connect 
        # need to let device warm up and flush out any old data
        self._freeSerialBuffer(ser)
        return ser 
    
    def findCorrectSerialDevice(self):
        _available_ports = serial.tools.list_ports.comports(include_links=False)
        ports_to_check = ["COM", "/dev/tt", "/dev/a", "/dev/cu.SLA", "/dev/cu.wchusb"]
        descriptions_to_check = ["CH340", "CP2102"]

        for port in _available_ports:
            if any(port.device.startswith(allowed_port) for allowed_port in ports_to_check) or \
               any(port.description.startswith(allowed_description) for allowed_description in descriptions_to_check):
                if self.tryToConnect(port.device):
                    return self.serialdevice

        self.is_connected = False
        self.serialport = "NotConnected"
        self.serialdevice = None
        self._parent.logger.debug("No USB device connected! Using DUMMY!")

    def tryToConnect(self, port):
        try:
            self.serialdevice = serial.Serial(port=port, baudrate=self.baudrate, timeout=1, write_timeout=1)
            self._freeSerialBuffer(self.serialdevice)
            if self.checkFirmware(self.serialdevice):
                self.is_connected = True
                self.NumberRetryReconnect = 0
                return True

        except Exception as e:
            self._parent.logger.debug(f"Trying out port {port} failed")
            self._parent.logger.error(e)

        return False

    def checkFirmware(self, ser):
        """Check if the firmware is correct"""
        path = "/state_get"
        payload = {"task": path}
        
        ser.write(json.dumps(payload).encode('utf-8'))
        ser.write(b'\n')

        for i in range(5):
            # if we just want to send but not even wait for a response
            mReadline = ser.readline()
            if mReadline.decode('utf-8').strip() == "++":
                self._freeSerialBuffer(ser)
                return True 
        return False            

    def _generate_identifier(self):
        self.identifier_counter += 1
        return self.identifier_counter

    def _process_commands(self):
        buffer = ""
        reading_json = False
        currentIdentifier = None
        nLineCountTimeout = 50 # maximum number of lines read before timeout 
        lineCounter = 0
        
        t0 = time.time()
        while self.running:
            # get the identifier just in case we cannot parse it from the serial
            currentIdentifier = self.getCurrentIdentfier()

            # device not ready yet
            if self.ser is None:
                continue
            
            # if we just want to send but not even wait for a response
            try:
                mReadline = self.ser.readline()
            except Exception as e:
                self._parent.logger.error(e)
                return 
            try:
                line = mReadline.decode('utf-8').strip()
                if self.DEBUG: print(line)
            except:
                line = ""
            if line == "++":
                reading_json = True
                continue
            elif line == "--" or lineCounter>nLineCountTimeout:
                lineCounter = 0
                reading_json = False
                try:
                    json_response = json.loads(buffer)
                except: 
                    print("Failed to load the json from serial")
                    json_response = {}            
                
                with self.lock:
                    try: currentIdentifier = json_response["qid"]
                    except: pass
                    try:
                        self.responses[currentIdentifier].append(json_response.copy())
                    except:
                        self.responses[currentIdentifier] = list()
                        self.responses[currentIdentifier].append(json_response.copy())
                    currentIdentifier = None
                buffer = ""     # reset buffer
                
            if reading_json:
                buffer += line
                lineCounter +=1 

            time.sleep(0.001)

    def get_json(self, path):
        message = {"task":path}
        message = json.dumps(message)
        return self.sendMessage(message, nResponses=0) 
    
    def post_json(self, path, payload, getReturn=True, nResponses=1):
        """Make an HTTP POST request and return the JSON response"""
        if payload is None:
            payload = {}
        if "task" not in payload:
            payload["task"] = path

        # write message to the serial
        if not getReturn:
            nResponses = -1
        writeResult = self.sendMessage(command=payload, nResponses=nResponses)
        return writeResult
    
    def writeSerial(self, payload):
        return self.sendMessage(payload, nResponses=-1)
    
    def breakCurrentCommunication(self):
        pass # not needed anymore
    
    def readSerial(self, qid=0, timeout=1):
        t0 = time.time()
        while time.time()-t0<timeout:
            try:
                return self.responses[qid]
            except:
                pass
        return {"timeout": 1}
        
    def sendMessage(self, command, nResponses=1, timeout=10):
        '''
        Sends a command to the device and optionally waits for a response.
        If nResponses is 0, then the command is sent but no response is expected.
        If nResponses is 1, then the command is sent and the response is returned.
        If nResponses is >1, then the command is sent and a list of responses is returned.        
        '''
        if type(command) == str:
            command = json.loads(command)
        identifier = self._generate_identifier()
        command["qid"] = identifier
        # trying to send message here instead:
        self.setCurrentIdentifier(identifier)
        if self.DEBUG: print("Sending: "+ str(command))
        with self.serial_lock:
            time.sleep(0.1)
            self.ser.write((json.dumps(command)+'\n').encode('utf-8'))
        self.commands[identifier]=command
        if nResponses <= 0:
            return identifier
        t0 = time.time()
        while self.running:
            time.sleep(0.002)
            if self.resetLastCommand:
                self.resetLastCommand = False
                return "communication interrupted"
            if time.time()-t0>timeout:
                return "timeout"
            with self.lock:
                if identifier in self.responses:
                    if len(self.responses[identifier])==nResponses:
                        return self.responses[identifier][-1]
                if -identifier in self.responses:
                    print("You have sent the wrong command!")
                    return "Wrong Command"
                    
            

    def stop(self):
        self.running = False
        self.thread.join()
        self.ser.close()
        
    def closeSerial(self):
        self.stop()
        
    def reconnect(self):
        if not self.isConnected:
            self.openDevice(self.port, self.baudrate)


if __name__ == "__main__":
    # Usage example
    monitor = Serial('/dev/cu.SLAB_USBtoUART', baudsrate=115200)  # Change to your port

    command_to_send = {
            "task": "/state_get"
    }

    command_to_send = {"task":"/motor_act","motor":{"steppers": [{ "stepperid": 3, "position": -1000, "speed": 15000, "isabs": 0, "isaccel":0}]}}
        
    t0 = time.time()
    response = monitor.sendMessage(command_to_send, nResponses=2)
    print("Response:", response)
    print("time:", time.time()-t0)

    t0 = time.time()
    response = monitor.sendMessage(command_to_send, nResponses=0)
    print("Response:", response)
    print("time:", time.time()-t0)


    #response = monitor.waitForResponse(command_id)

    if response:
        print("Response:", response)

    monitor.stop()


class SerialManagerWrapper:
    
    def __init__(self) -> None:
        pass