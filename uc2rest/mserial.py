import serial
import json
import queue
import threading
import time

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
        self.command_queue = queue.Queue()
        self.responses = {}
        self.commands = {}
        self.lock = threading.Lock()
        self.running = True
        self.identifier_counter = 0 # Counter for generating unique identifiers
        self.thread = threading.Thread(target=self._process_commands)
        self.thread.start()

        # initialize serial connection        
        self.ser = self.openDevice(port, baudrate, timeout)
        

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
        ser = serial.Serial(port, baud_rate, timeout=1)
        self.is_connected = True
        ser.write_timeout = 1    
        # TODO: Need to be able to auto-connect 
        # need to let device warm up and flush out any old data
        self._freeSerialBuffer(ser)
        return ser 
    
    def checkFirmware(self, timeout=1):
        pass

    def _generate_identifier(self):
        self.identifier_counter += 1
        return self.identifier_counter

    def _process_commands(self):
        buffer = ""
        reading_json = False
        currentIdentifier = None
        waitForResponse = False
        readTimeout = 0

        t0 = time.time()
        while self.running:
            if not self.command_queue.empty():
                currentIdentifier, command, readTimeout = self.command_queue.get()
                t0 = time.time()
                json_command = json.dumps(command)
                self.ser.write(json_command.encode('utf-8'))
                self.ser.write(b'\n')

            # device not ready yet
            if self.ser is None:
                continue
            
            # if we just want to send but not even wait for a response
            mReadline = self.ser.readline()
            try:
                line = mReadline.decode('utf-8').strip()
                if self.DEBUG: print(line)
            except:
                line = ""
            if line == "++":
                reading_json = True
                continue
            elif line == "--" :
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
                buffer = ""     # reset buffer
                t0 = time.time() # reset timeout
                continue
            elif time.time()-t0 > readTimeout: # todo: should be currrent_identifier specific
                reading_json = False
                with self.lock:
                    self.responses[currentIdentifier] = "timeout"
                buffer = ""
                continue

            if reading_json:
                buffer += line

            time.sleep(0.001)

    def get_json(self, path):
        message = {"task":path}
        message = json.dumps(message)
        return self.sendMessage(message, readTimeout=1, nResponses=0) 
    
    def post_json(self, path, payload, getReturn=True, nResponses=1, timeout=1):
        """Make an HTTP POST request and return the JSON response"""
        if payload is None:
            payload = {}
        if "task" not in payload:
            payload["task"] = path

        # write message to the serial
        if not getReturn:
            nResponses = -1
        writeResult = self.sendMessage(command=payload, readTimeout=timeout, nResponses=nResponses)
        return writeResult
    
    def writeSerial(self, payload):
        self.sendMessage(payload, nResponses=-1)
    
    def breakCurrentCommunication(self):
        pass # not needed anymore
    
    def readSerial(self, is_blocking=True, timeout = 1):
        return self.responses
        
    def sendMessage(self, command, readTimeout=1, nResponses=1):
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
        self.command_queue.put((identifier, command, readTimeout))
        self.commands[identifier]=command
        if nResponses <= 0:
            return ""
        while self.running:
            time.sleep(0.005)
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