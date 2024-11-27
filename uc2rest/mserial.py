import threading
import serial
from serial.tools import list_ports
import time
import json
import queue
from collections import deque
from .MockSerial import MockSerial
import logging
        
T_SERIAL_WARMUP = .5


class RingBuffer(deque):
    def __init__(self, size_max):
        super().__init__(maxlen=size_max)

    def append(self, datum):
        super().append(datum)
        return self

    def get(self):
        return list(self)
    
    
class Serial:
    def __init__(self, port, baudrate=115200, timeout=5,
                 identity="UC2_Feather", parent=None, DEBUG=False):

        '''
        serial_device is the serial object that can read/write
        serial_port_name is the name of the port which is open or to be opened
        '''

        self.baudrate = baudrate        # Baud rate for serial communication
        self.timeout = timeout          # Timeout for reading from serial port
        self.identity = identity        # Identity of the device (e.g. UC2_Feather)
        self.DEBUG = DEBUG              # Debug flag
        self._parent = parent            # Parent object
        self.serial_port_name = port    # Serial port name
        self.serial_device = None       # Serial device object 
        self.is_connected = False       # Flag to indicate if the device is connected
        self.resetLastCommand = False   # Flag to reset wiating of the last command
        self.data_queue = queue.Queue() # Queue to store incoming data
        self.maxQentries = 100          # Maximum number of entries in the queue
        self.queueFinalizedQueryIDs = RingBuffer(self.maxQentries)
        self.identifier_counter = 0     # Counter for generating unique identifiers
        self.responses = {}             # Dictionary to store responses from the esp per QID
        self.serial_write_lock = threading.Lock()  # Lock for writing to serial port
        self.isReadingLoopRunning = False          # Flag to indicate if the serial port is being read
        self.isWritingLoopRunning = False           # Flag to indicate if the serial port is being written to
        self.read_timeout = 0.02        # Timeout for reading from serial port
        self.write_timeout = 1      
        self.cmdCallBackFct = None      # in case we want to route the commands to an external function     
        # setup callback list for parent modules
        self.callBackList = []
        
        # get hold on the logger
        if self._parent is None:
            self._logger = logging.getLogger(__name__)
            self._logger.setLevel(logging.DEBUG)
            self._logger.addHandler(logging.StreamHandler())
        else:
            self._logger = self._parent.logger


        # try to open the port
        self.open(port = self.serial_port_name, baudrate=self.baudrate)
        
        
    def closeDevice(self):
        '''
        Close the serial port
        '''
        if self.serial_port_name is not None:
            try:
                self.serial_device.close()
            except Exception as e:
                self._logger.error("[CloseDevice]: "+str(e))
            
    def open(self, port=None, baudrate=None):
        '''
        Open the serial port
        '''
        if baudrate is None:
            baudrate = self.baudrate
        self.serial_device = self.openDevice(port=port, baudrate=baudrate)
        self.is_connected = True
        self.start_reading()
        
    def close(self):
        ''' 
        Stop threads adn close the serial port
        '''
        self.stop_reading()
        self.closeDevice()
        
        
    def reconnect(self):
        '''
        Reconnect the serial port
        '''
        self.close()
        self.open(port=self.serial_port_name, baudrate=self.baudrate)
        
    def openDevice(self, port=None, baudrate=115200):
        '''
        Open the serial device and check if it is a UC2 device
        If port is known, we'll try that first, otherwisse we'll scan all available ports
        
        it returns the opened serial object
        
        The order is as follows:
        0. Try to close the serial port if it is open
        1. Try to connect to the given port
        2. If the firmware is correct, return the serial object
        3. If the firmware is incorrect, try to connect to the correct port by scanning all available ports
        4. If the firmware is correct, return the serial object
        5. If the firmware is incorrect, return a MockSerial object
        6. If no USB device is connected, return a MockSerial object        
        '''
        
        # try to close an eventually open serial connection
        if hasattr(self, "serial_port_name") and self.serial_device is not None and str(type(self.serial_device)) != "<class 'uc2rest.mserial.MockSerial'>":
            self.closeDevice()
        
        # first try to connect to a given port 
        if port is not None:
            isUC2, serial_device = self._try_to_connect(port=port)
            if isUC2:
                self.is_connected = True
        # if not successful scan for alternative ports
        if port is None or not isUC2:
            serial_device = self._find_correct_serial_device()
            if serial_device is None:
                serial_device = MockSerial(port, baudrate, timeout=.1)
                self.is_connected = False

        # TODO: Need to be able to auto-connect
        # need to let device warm up and flush out any old data
        #self.freeSerialBuffer(serial_device)
        return serial_device
    
    def closeDevice(self):
        '''
        Close the serial port
        '''
        if self.serial_port_name is not None:
            try:
                self.serial_device.close()
            except Exception as e:
                self._logger.error("[CloseDevice]: "+str(e))
                
    def _find_correct_serial_device(self):
        '''
        This function tries to find the correct serial device from the list of available ports
        It also checks if the firmware is correct by sending a command and checking for the correct response
        It may be that - depending on the OS - the response may be corrupted
        If this is the case try to hard-code the COM port into the config JSON file
        '''
        _available_ports = list_ports.comports(include_links=False)
        ports_to_check = ["COM", "/dev/tt", "/dev/a", "/dev/cu.SLA", "/dev/cu.wchusb"]
        descriptions_to_check = ["CH340", "CP2102"]

        for port in _available_ports:
            if any(port.device.startswith(allowed_port) for allowed_port in ports_to_check) or \
               any(port.description.startswith(allowed_description) for allowed_description in descriptions_to_check):
                isUC2, serialdevice = self._try_to_connect(port.device)
                if isUC2:
                    self.is_connected = True
                    self.manufacturer = port.manufacturer
                    return serialdevice

        self.is_connected = False
        self.serialport = "NotConnected"
        self.serialdevice = None
        self._logger.debug("No USB device connected! Using DUMMY!")
        self.manufacturer = "UC2Mock"
        return None

    def _free_serial_buffer(self, ser, timeout=5, timeMinimum=0):
        t0 = time.time()
        # free up any old data
        while True:
            try:
                readLineRaw = ser.readline()
                readLine = readLineRaw.decode('utf-8').strip()
                if self.DEBUG and readLine != "": 
                    self._logger.debug(readLine)
                if readLine == "" and time.time()-t0 > timeMinimum:
                    break
            except Exception as e:
                if self.DEBUG: self._logger.error("[FreeSerial]: "+str(e))

                pass
            if time.time()-t0 > timeout:
                return

    def _try_to_connect(self, port):
        try:
            serialdevice = serial.Serial(port=port, baudrate=self.baudrate, timeout=self.read_timeout, write_timeout=self.write_timeout)
            # close the device - similar to hard reset
            serialdevice.setDTR(False)
            serialdevice.setRTS(True)
            time.sleep(.1)
            serialdevice.setDTR(False)
            serialdevice.setRTS(False)
            time.sleep(.5)            
            #time.sleep(T_SERIAL_WARMUP)
            self._free_serial_buffer(serialdevice, timeout=2, timeMinimum=1)
            if self._check_firmware(serialdevice):
                self.is_connected = True
                self.NumberRetryReconnect = 0
                return True, serialdevice
            else:
                False, None

        except Exception as e:
            self._logger.debug(f"Trying out port {port} failed: "+str(e))

        return False, None

    def _check_firmware(self, ser):
        """Check if the firmware is correct
        We do not do that inside the queue processor yet
        """
        path = "/state_get"
        payload = {"task": path}

        ser.write(json.dumps(payload).encode('utf-8'))
        ser.write(b'\n')
        # iterate a few times in case the debug mode on the ESP32 is turned on and it sends additional lines
        for i in range(700):
            # if we just want to send but not even wait for a response
            mReadline = ser.readline()
            if self.DEBUG and mReadline != "" and mReadline != "\n" and mReadline != b'' and mReadline != b'\n': 
                self._logger.debug("[_check_firmware]: "+str(mReadline))
            if mReadline.decode('utf-8').strip() == "++":
                self._free_serial_buffer(ser)
                return True
        return False
            
    def _generate_identifier(self):
        '''
        Generate a unique identifier for the communication for any command to send
        '''
        self.identifier_counter += 1
        return self.identifier_counter % 2**12

    def breakCurrentCommunication(self):
        self.resetLastCommand = True
        
    def start_reading(self):
        """
        Start reading serial port in a separate thread.
        
        The read_thread will continously poll the serial port for incoming data and
        the worker_thread will process the incoming data e.g. parse JSON objects.
        """
        if self.is_connected:
            if not self.isReadingLoopRunning:
                self.read_thread = threading.Thread(target=self._read_loop)
                self.read_thread.start()
            if not self.isWritingLoopRunning:
                self.worker_thread = threading.Thread(target=self._process_data)
                self.worker_thread.start()
        

    def stop_reading(self):
        """Stop reading loop and close serial port."""
        if self.is_connected:
            self.is_connected = False
            self.read_thread.join()
        if self.serial_device.is_open:
            self.serial_device.close()


    def extract_json_objects(self, s):
        """
        Extract JSON objects from a string that starts with '++' and ends with '--',
        and return the remaining string after the last successful JSON extraction.
        
        Parameters:
        - s (str): The input string containing JSON objects.
        
        Returns:
        - List[dict]: A list of dictionaries parsed from the JSON objects in the string.
        - str: The remaining part of the string after the last successful JSON object extraction.
        """
        parts = s.split('++')
        dictionaries = []
        last_position = 0  # Track the position in the original string
        remainder = ""  # Track the remaining string after the last successful JSON extraction
        for part in parts:
            # Attempt to extract the JSON string
            json_str = part.split('--')[0].strip()
            if json_str:
                try:
                    # Update the position tracker to the end of the current part
                    last_position += len(part) + 2  # +2 accounts for the '++' delimiter
                    json_dict = json.loads(json_str)
                    dictionaries.append(json_dict)
                except json.JSONDecodeError as e:
                    try:
                        #trying to repair the string
                        import re
                        match = re.search(r'"qid":(\d+)', json_str)
                        if match:
                            qid = int(match.group(1)) 
                            json_dict = {"qid": qid}
                            dictionaries.append(json.loads(json.dumps(json_dict)))
                    except Exception as e:
                        self._logger.debug(f"Failed to decode JSON: {json_str}. Error: {e}")
                        last_position = last_position -( len(part) + 2)
                        #break  # Stop processing further on error
        
                        # Extract the remaining string from the last successful position to the end
                        remainder += s[last_position:]
    
        return dictionaries, remainder


    def _read_loop(self):
        """Read data from serial port and add it to the queue."""
        self.isReadingLoopRunning = True
        while self.is_connected:
            if self.serial_device.in_waiting:
                #with self.serial_io_lock:
                data = self.serial_device.read(self.serial_device.in_waiting)
                self.data_queue.put(data)
            time.sleep(0.03)  # Short delay to prevent CPU overuse
        self.isReadingLoopRunning = False
        
    def _process_data(self):
        """Process data in a separate thread."""
        accumulatedRemainder = ""
        self.isWritingLoopRunning = True
        while self.is_connected:
            data = self.data_queue.get()
            try:
                #self._logger.debug(data.decode('utf-8'), end='', flush=True)
                data = data.decode('utf-8')
                data = data.replace('\t', '').replace('\n', '').replace('\r', '') # Remove whitespace characters - better formatting should do better?
                
                # detect a reboot of the device and return the current QIDs
                if data.find("reboot") >= 0:
                    self._logger.warning("Device rebooted")
                    self.resetLastCommand = True
                    continue
                
                dictionaries, remainder = self.extract_json_objects(accumulatedRemainder + data)
                accumulatedRemainder += remainder
                self._logger.debug("Accumulated Remainder:"+accumulatedRemainder)
                self._logger.debug(dictionaries)
                for dictionary in dictionaries:
                    if "qid" in dictionary:
                        self.queueFinalizedQueryIDs.append(dictionary["qid"])
                        
                        # add the response to the dictionary
                        if dictionary["qid"] in self.responses:
                            self.responses[dictionary["qid"]].append(dictionary)
                        else:
                            self.responses[dictionary["qid"]] = [dictionary]
                            
                        if self.DEBUG: self._logger.debug(f"Received response for query ID: {dictionary['qid'], dictionary}")
                        if dictionary["qid"]<0 and self.DEBUG: self._logger.error("You have sent the wrong command! Maybe module not active?")
                        
                        if len(self.callBackList) > 0:
                            for callback in self.callBackList:
                                # check if json has key
                                try:
                                    if callback["pattern"] in dictionary:
                                        callback["callbackfct"](dictionary)
                                except Exception as e:
                                    self._logger.error("[ProcessCommands]: "+str(e))

                    else:
                        self._logger.debug(f"Dictionary does not contain 'qid': {dictionary}")

            except Exception as e:
                self._logger.debug(f"Failed to decode data: {data}. Error: {e}")
            time.sleep(0.05)  # Short delay to prevent CPU overuse
        self.isWritingLoopRunning = False
                
    def register_callback(self, callback, pattern):
        '''
        we need to add a callback function to a list of callbacks that will be read during the serial communication
        loop
        '''
        self.callBackList.append({"callbackfct":callback, "pattern":pattern})

    def write_data(self, data: str):
        """Send data to the serial port."""
        with self.serial_write_lock:
            if self.DEBUG: self._logger.debug(f"Writing data: {data}")
            self.serial_device.write(data.encode('utf-8'))
            self.serial_device.flush() # Ensure data is sent immediately
            
    def sendMessage(self, data:str, nResponses: int=1, mTimeout:float=10., blocking:bool=True):
        '''
        Send a message to the serial port and wait or do not wait for the response
        
        Sends a command to the device and optionally waits for a response.
        If nResponses is 0, then the command is sent but no response is expected.
        If nResponses is 1, then the command is sent and the response is returned.
        If nResponses is >1, then the command is sent and a list of responses is returned.
        '''
        # if no qid can be assigned to the return message, do not block
        # if the data is a string, convert it to a dictionary
        if type(data) == str:
            data = json.loads(data)
        try:
            cqid = data["qid"]
            self.identifier_counter = cqid
        except: 
            cqid = self._generate_identifier()
            data["qid"] = cqid
        if self.DEBUG: self._logger.debug(f"Sending message: {cqid}, blocking: {blocking}, message length: {len(data)}")
        self.write_data(json.dumps(data))
        # wait for the response
        cTime = time.time()
        
        if nResponses <= 0 or mTimeout <= 0 or blocking == False:
            blocking = False
            time.sleep(0.05) # short delay to prevent CPU overuse
            return cqid
        
        while blocking:
            try:
                if time.time() - cTime > mTimeout:
                    self._logger.debug(f"Timeout of {mTimeout} seconds reached.")
                    break
                # compare with any received responses
                qids = self.queueFinalizedQueryIDs.get()
                if cqid in qids and qids.count(cqid) >= nResponses:
                    return self.responses[cqid]
                if -cqid in qids:
                    self._logger.debug("You have sent the wrong command!")
                    return "Wrong Command"
            except queue.Empty:
                #self._logger.debug("Waiting for response...")
                pass
            time.sleep(0.05) # short delay to prevent CPU overuse

    def get_json(self, path, timeout=1):
        message = {"task":path}
        message = json.dumps(message)
        return self.sendMessage(message, nResponses=0, timeout=timeout)

    def writeSerial(self, payload):
        '''wrapper function to side-load a self-constructed JSON string'''
        return self.sendMessage(payload, nResponses=-1)
    
    def readSerial(self, qid=0, timeout=1):
        '''enforce to read the result of a certain QID'''
        t0 = time.time()
        while time.time()-t0<timeout:
            try:
                return self.responses[qid]
            except:
                pass
        return {"timeout": 1}
    
    def post_json(self, path, payload, getReturn=True, nResponses=1, timeout=100):
        """Make an HTTP POST request and return the JSON response"""
        if payload is None:
            payload = {}
        if "task" not in payload:
            payload["task"] = path

        # write message to the serial
        if not getReturn:
            nResponses = -1
        if self.cmdCallBackFct is not None: # TODO: what did we need this for?!
            self.cmdCallBackFct(payload)
            return "OK"
        else:
            writeResult = self.sendMessage(data=payload, nResponses=nResponses, mTimeout=timeout, blocking=getReturn)
            return writeResult

          
    def toggleCommandOutput(self, cmdCallBackFct=None):
        # if true, all commands will be output to a callback function and stored for later use
        self.cmdCallBackFct = cmdCallBackFct
        
        
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

    def flush(self):
        pass

    def isOpen(self):
        return self.is_open

    def open(self):
        self.is_open = True

    def close(self):
        self.is_open = False
        
    def in_waiting(self):
        return

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
        while self.is_open:
            if random.random() < 0.2:  # Simulate occasional data availability
                self.data_buffer.extend([random.randint(0, 255) for _ in range(10)])
            time.sleep(0.1)

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()


# Example of using SimpleSerialComm
#python /Users/bene/mambaforge/envs/imswitch/lib/python3.9/site-packages/serial/tools/miniterm.py /dev/cu.SLAB_USBtoUART 500000

if __name__ == '__main__':
    port = "/dev/cu.SLAB_USBtoUART"
    baudrate = 115200# 500000
    mSerial = Serial(port, baudrate, DEBUG=1)
    
    cqid = 0
    mTimeout = 10
    while True:
        
        # short test
        message = '{"task":"/motor_act", "motor": { "steppers": [ { "stepperid": 1, "position": 1000, "speed": 5000, "isabs": 0, "isaccel":0}, { "stepperid": 2, "position": 1000, "speed": 5000, "isabs": 0, "isaccel":0}]}, "qid":'+str(cqid)+'}' 
        mSerial.sendMessage(data=message, mTimeout=10)
        cqid += 1
        
        # long test split  
        message1 = '{"task": "/ledarr_act", "led": {"LEDArrMode": 0, "led_array": [{"id": 0, "r": 22, "g": 45, "b": 29}, {"id": 1, "r": 12, "g": 34, "b": 22}, {"id": 2, "r": 34, "g": 11, "b": 50}, {"id": 3, "r": 32, "g": 22, "b": 2}, {"id": 4, "r": 37, "g": 1, "b": 35}, {"id": 5, "r": 21, "g": 10, "b": 28}, {"id": 6, "r": 52, "g": 16, "b": 5}, {"id": 7, "r": 2, "g": 43, "b": 29}, {"id": 8, "r": 42, "g": 2, "b": 39}, {"id": 9, "r": 21, "g": 21, "b": 22}, {"id": 10, "r": 44, "g": 28, "b": 35}, {"id": 11, "r": 31, "g": 40, "b": 52}, {"id": 12, "r": 25, "g": 45, "b": 15}, {"id": 13, "r": 20, "g": 24, "b": 1}, {"id": 14, "r": 49, "g": 48, "b": 37}, {"id": 15, "r": 54, "g": 3, "b": 41}, {"id": 16, "r": 14, "g": 17, "b": 16}, {"id": 17, "r": 48, "g": 31, "b": 47}, {"id": 18, "r": 43, "g": 24, "b": 10}, {"id": 19, "r": 23, "g": 28, "b": 54}]}, "qid": '+str(cqid)+'}'
        cqid += 1
        message2 = '{"task": "/ledarr_act", "led": {"LEDArrMode": 0, "led_array": [{"id": 28, "r": 18, "g": 6, "b": 28}, {"id": 29, "r": 14, "g": 47, "b": 5}, {"id": 30, "r": 2, "g": 31, "b": 24}, {"id": 31, "r": 53, "g": 8, "b": 50}, {"id": 32, "r": 1, "g": 15, "b": 14}, {"id": 33, "r": 19, "g": 18, "b": 39}, {"id": 34, "r": 44, "g": 22, "b": 29}, {"id": 35, "r": 4, "g": 38, "b": 9}, {"id": 36, "r": 1, "g": 28, "b": 54}, {"id": 37, "r": 37, "g": 43, "b": 22}, {"id": 38, "r": 14, "g": 27, "b": 27}, {"id": 39, "r": 22, "g": 10, "b": 49}, {"id": 40, "r": 32, "g": 29, "b": 39}, {"id": 41, "r": 14, "g": 2, "b": 41}, {"id": 42, "r": 39, "g": 37, "b": 35}, {"id": 43, "r": 26, "g": 44, "b": 32}, {"id": 44, "r": 45, "g": 19, "b": 53}, {"id": 45, "r": 44, "g": 26, "b": 37}, {"id": 46, "r": 40, "g": 28, "b": 11}, {"id": 47, "r": 9, "g": 4, "b": 23}, {"id": 48, "r": 41, "g": 22, "b": 31}, {"id": 49, "r": 10, "g": 5, "b": 46}, {"id": 50, "r": 48, "g": 39, "b": 52}, {"id": 51, "r": 33, "g": 15, "b": 26}, {"id": 52, "r": 50, "g": 19, "b": 44}, {"id": 53, "r": 34, "g": 18, "b": 35}, {"id": 54, "r": 6, "g": 0, "b": 39}, {"id": 55, "r": 6, "g": 26, "b": 43}, {"id": 56, "r": 24, "g": 35, "b": 21}, {"id": 57, "r": 47, "g": 8, "b": 31}, {"id": 58, "r": 1, "g": 0, "b": 32}, {"id": 59, "r": 52, "g": 12, "b": 28}, {"id": 60, "r": 39, "g": 53, "b": 5}, {"id": 61, "r": 6, "g": 32, "b": 41}, {"id": 62, "r": 28, "g": 0, "b": 24}, {"id": 63, "r": 34, "g": 46, "b": 27}]}, "qid": '+str(cqid)+'}'
        cqid += 1
        
        messages = [message1, message2]
        
        for message in messages:
            mSerial.sendMessage(data=message, blocking=True, mTimeout=1)
        
        # very short test
        message = '{"task": "/state_get", "qid": '+str(cqid)+'}'
        mSerial.sendMessage(data=message, blocking=False)
        cqid += 1
            