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
        self.write_timeout = 0.02

        self.cmdCallBackFct = None

        # setup command queue
        self.resetLastCommand = False
        self.command_queue = queue.Queue()
        self.responses = {}
        self.commands = {}
        self.lock = threading.Lock()

        self.callBackList = []

        # initialize serial connection
        self.thread = None
        self.ser = self.openDevice(port, baudrate, timeout)

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

    def openDevice(self, port=None, baud_rate=115200, timeout=5):
        try:
            ser = serial.Serial(port, baud_rate, timeout=.01)
            ser.write_timeout = self.write_timeout
            self.is_connected = True
        except Exception as e:
            self._parent.logger.error(e)
            ser = self.findCorrectSerialDevice()
            if ser is None:
                ser = MockSerial(port, baud_rate, timeout=.1)
                self.is_connected = False
        ser.write_timeout = self.write_timeout
        if not ser.isOpen(): ser.open()
        # TODO: Need to be able to auto-connect
        # need to let device warm up and flush out any old data
        self._freeSerialBuffer(ser)

        # remove any remaining thread in case there was one open
        try:
            del self.thread
        except:
            pass
        self.running = True
        self.identifier_counter = 0 # Counter for generating unique identifiers
        self.thread = threading.Thread(target=self._process_commands)
        self.thread.start()
        return ser

    def findCorrectSerialDevice(self):
        _available_ports = serial.tools.list_ports.comports(include_links=False)
        ports_to_check = ["COM", "/dev/tt", "/dev/a", "/dev/cu.SLA", "/dev/cu.wchusb"]
        descriptions_to_check = ["CH340", "CP2102"]

        for port in _available_ports:
            if any(port.device.startswith(allowed_port) for allowed_port in ports_to_check) or \
               any(port.description.startswith(allowed_description) for allowed_description in descriptions_to_check):
                if self.tryToConnect(port.device):
                    self.is_connected = True
                    return self.serialdevice

        self.is_connected = False
        self.serialport = "NotConnected"
        self.serialdevice = None
        self._parent.logger.debug("No USB device connected! Using DUMMY!")

    def tryToConnect(self, port):
        try:
            self.serialdevice = serial.Serial(port=port, baudrate=self.baudrate, timeout=1, write_timeout=self.write_timeout)
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
            if not self.command_queue.empty() and not reading_json:
                currentIdentifier, command = self.command_queue.get()
                if self.DEBUG: self._parent.logger.debug("Sending: "+ str(command))
                json_command = json.dumps(command)
                try:
                    self.ser.write(json_command.encode('utf-8'))
                except Exception as e:
                    try:
                        self.ser.write_timeout = 1
                        self.ser.write(json_command.encode('utf-8'))
                        self.ser.write_timeout=self.write_timeout
                    except Exception as e:
                        self._parent.logger.error(e)
                try:self.ser.write(b'\n')
                except: break
            # device not ready yet
            if self.ser is None:
                self.is_connected = False
                continue
            else:
                self.is_connected = True

            # if we just want to send but not even wait for a response
            try:
                mReadline = self.ser.readline()
            except Exception as e:
                self._parent.logger.error(e)
                self.is_connected = False
                break
            try:
                line = mReadline.decode('utf-8').strip()
                if self.DEBUG and line!="": self._parent.logger.debug(line)
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
                    if len(self.callBackList) > 0:
                        for callback in self.callBackList:
                            # check if json has key
                            try: 
                                if callback["pattern"] in json_response:
                                    callback["callbackfct"](json_response)    
                            except Exception as e:
                                self._parent.logger.debug(e)
                            
                except: 
                    self._parent.logger.debug("Failed to load the json from serial")
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

            if reading_json:
                buffer += line
                lineCounter +=1
        self.running = False

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
        if self.cmdCallBackFct is not None:
            self.cmdCallBackFct(payload)
            return "OK"
        else:
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

    def register_callback(self, callback, pattern):
        '''
        we need to add a callback function to a list of callbacks that will be read during the serial communication
        loop
        ''' 
        self.callBackList.append({"callbackfct":callback, "pattern":pattern})
        
    def sendMessage(self, command, nResponses=1, timeout = 20):
        '''
        Sends a command to the device and optionally waits for a response.
        If nResponses is 0, then the command is sent but no response is expected.
        If nResponses is 1, then the command is sent and the response is returned.
        If nResponses is >1, then the command is sent and a list of responses is returned.
        '''
        t0 = time.time()
        if type(command) == str:
            command = json.loads(command)
        identifier = self._generate_identifier()
        command["qid"] = identifier
        self.command_queue.put((identifier, command))
        self.commands[identifier]=command
        if nResponses <= 0 or not self.is_connected or not type(self.ser.BAUDRATES) is tuple:
            return identifier
        while self.running:
            time.sleep(0.002)
            if self.resetLastCommand or time.time()-t0>timeout or not self.is_connected:
                self.resetLastCommand = False
                return "communication interrupted"
            with self.lock:
                if identifier in self.responses:
                    if len(self.responses[identifier])==nResponses:
                        return self.responses[identifier][-1]
                if -identifier in self.responses:
                    self._parent.logger.debug("You have sent the wrong command!")
                    return "Wrong Command"

    def interruptCurrentSerialCommunication(self):
        self.resetLastCommand = True

    def stop(self):
        self.running = False
        self.thread.join()
        self.ser.close()

    def closeSerial(self):
        self.stop()

    def reconnect(self):
        self.running=0
        try:
            self.ser.close()
        except:
            pass
        self.openDevice(port = self.serialport, baud_rate = self.baudrate)

    def toggleCommandOutput(self, cmdCallBackFct=None):
        # if true, all commands will be output to a callback function and stored for later use
        self.cmdCallBackFct = cmdCallBackFct

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

import random
from threading import Thread
class MockSerial:
    def __init__(self, port, baudrate, timeout=1):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.is_open = False
        self.data_buffer = []
        self.thread = Thread(target=self._simulate_data)
        self.thread.daemon = True
        self.thread.start()
        self.is_open = True
        self.manufacturer = "UC2Mock"
        self.BAUDRATES = -1

    def isOpen(self):
        return self.is_open 
    
    def open(self):
        self.is_open = True

    def close(self):
        self.is_open = False

    def readline(self, timeout=1):
        if not self.is_open:
            raise Exception("Device not connected")
        if len(self.data_buffer) == 0:
            return b''
        data = self.data_buffer
        self.data_buffer = self.data_buffer
        time.sleep(.05)
        return bytes(data)

    def read(self, num_bytes):
        if not self.is_open:
            raise Exception("Device not connected")
        if len(self.data_buffer) == 0:
            return b''
        data = self.data_buffer[:num_bytes]
        self.data_buffer = self.data_buffer[num_bytes:]
        return bytes(data)

    def write(self, data):
        if not self.is_open:
            raise Exception("Device not connected")
        pass  # Do nothing, as it's a mock

    def _simulate_data(self):
        while True:
            if self.is_open:
                if random.random() < 0.2:  # Simulate occasional data availability
                    self.data_buffer.extend([random.randint(0, 255) for _ in range(10)])
            time.sleep(0.1)

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
