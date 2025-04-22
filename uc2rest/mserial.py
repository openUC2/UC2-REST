import serial
import json
import queue
import threading
import time
import sys
try:
    from serial.tools  import list_ports
    from serial.serialutil import SerialException
    IS_SERIAL = True
except ImportError:
    IS_SERIAL = False
    class SerialException(Exception):
        pass

T_SERIAL_WARMUP = 2.5
class Serial:
    def __init__(self, port, baudrate=115200, timeout=5,
                 identity="UC2_Feather", parent=None, DEBUG=False, 
                 skipFirmwareCheck=False):

        self.serialdevice = None
        self.serialport = port
        self.baudrate = baudrate
        self.timeout = timeout
        self._parent = parent
        self.manufacturer = ""
        self.skipFirmwareCheck = skipFirmwareCheck
        if self._parent is None:
            import logging
            self._logger = logging.getLogger(__name__)
            self._logger.setLevel(logging.DEBUG)
            self._logger.addHandler(logging.StreamHandler())
        else:
            self._logger = self._parent.logger

        self.identity = identity
        self.DEBUG = DEBUG
        self.is_connected = False
        self.write_timeout = 1
        self.read_timeout = 0.02
        self.timeReturnReceived = 1#  0.5

        self.cmdWriteCallBackFct = None
        self.cmdReadCallBackFct = None

        # setup command queue
        self.resetLastCommand = False
        self.sender_queue = queue.Queue()
        self.responses = {}
        self.commands = {}
        self.lock = threading.Lock()
        self.serialLock = threading.Lock()
        
        # setup callback list for parent modules
        self.callBackList = []

        # initialize serial connection
        self.thread = None
        if IS_SERIAL:
            self.serialdevice= self.openDevice(port, baudrate)
        else: 
            ''' this is most likely happening because we work under pyoidide '''
            self._logger.debug("You have to provide the serial interface on your own")

    def breakCurrentCommunication(self):
        self.resetLastCommand = True

    def _freeSerialBuffer(self, ser, timeout=5, timeMinimum=0):
        t0 = time.time()
        # free up any old data
        while True:
            try:
                readLineRaw = self._read(ser)   
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


    def get_port_info(self, port_name):
        """
        Überprüft, ob ein Portname in der Liste der verfügbaren Ports enthalten ist,
        und gibt die zugehörigen Portinformationen zurück.

        :param port_name: Der Name des Ports (z. B. '/dev/ttyUSB0')
        :return: Portinformationen oder None, wenn der Port nicht gefunden wurde
        """
        available_ports = list_ports.comports()
        for port in available_ports:
            if port.device == port_name:
                return port  # Gibt die Portinformationen zurück
        return None  # Port nicht gefunden


    def openDevice(self, port=None, baud_rate=115200):
        try: # try to close an eventually open serial connection
            if str(type(self.ser)) != "<class 'uc2rest.mserial.MockSerial'>":
                self.serialdevice.close()
        except: pass

        
        try:
            # check if port is string and if so within available ports 
            if type(port)==str:
                port = self.get_port_info(port)
                if port is None:
                    raise ValueError("Port not found")
                
            for i in range(2): # not good, but sometimes it  needs a second attempt
                isUC2 = self.tryToConnect(port)
                if isUC2:
                    break
            if not isUC2:
                raise ValueError('Wrong Firmware.')
            ser = self.serialdevice
            self.is_connected = True

        except Exception as e:
            self._logger.error("[OpenDevice]: "+str(e))
            ser = self.findCorrectSerialDevice()
            if ser is None:
                ser = MockSerial(port, baud_rate, timeout=.1)
                self.is_connected = False
        ser.write_timeout = self.write_timeout
        if not ser.isOpen():
            ser.open()
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
        self.thread = threading.Thread(target=self._process_commands, daemon=True)
        self.thread.start()
        
        # setup sender queue
        self.sender_worker_thread = threading.Thread(target=self._sending_commands)
        self.sender_worker_thread.daemon = True  # Ensure the thread exits when the main program does
        self.sender_worker_thread.start()  
        return ser

    def findCorrectSerialDevice(self):
        '''
        This function tries to find the correct serial device from the list of available ports
        It also checks if the firmware is correct by sending a command and checking for the correct response
        It may be that - depending on the OS - the response may be corrupted
        If this is the case try to hard-code the COM port into the config JSON file
        '''
        _available_ports = list_ports.comports(include_links=False)
        current_os = sys.platform.lower()

        # OS-specific port prefixes and descriptions
        if current_os.startswith("win"):
            ports_to_check = ["COM"]
            descriptions_to_check = ["CH340", "CP2102", "USB Serial"]
        elif current_os.startswith("darwin"):
            # prefer cu.SLAB* over others on mac
            ports_to_check = ["/dev/cu.SLAB", "/dev/cu.wchusb", "/dev/cu.usbmodem"]
            descriptions_to_check = ["CH340", "CP2102", "USB-Serial"]
        else:  # linux or other
            ports_to_check = ["/dev/ttyUSB", "/dev/ttyACM"]
            descriptions_to_check = ["CH340", "CP2102", "USB2.0-Serial", "USB-Serial"]

        for port in _available_ports:
            if any(port.device.startswith(p) for p in ports_to_check) or \
            any(port.description.startswith(d) for d in descriptions_to_check):
                if current_os.startswith("darwin") and port.device.startswith("/dev/cu.usbserial-"):
                    continue
                if self.tryToConnect(port):
                    self.is_connected = True
                    self.manufacturer = port.manufacturer
                    return self.serialdevice

        self.is_connected = False
        self.serialport = "NotConnected"
        self.serialdevice = None
        self._logger.debug("No USB device connected! Using DUMMY!")
        self.manufacturer = "UC2Mock"
        return None

    def tryToConnect(self, port):
        try:
            self.serialdevice = serial.Serial(port.device, baudrate=self.baudrate, timeout=self.read_timeout, write_timeout=self.write_timeout)
            # close the device - similar to hard reset
            if not port.description == "USB JTAG/serial debug unit": # ESP32S3 won't work like that -> non configured device afterwards 
                self.serialdevice.setDTR(False)
                self.serialdevice.setRTS(True)
                time.sleep(.1)
                self.serialdevice.setDTR(False)
                self.serialdevice.setRTS(False)
                time.sleep(.5)            
            #time.sleep(T_SERIAL_WARMUP)
            self._freeSerialBuffer(self.serialdevice, timeout=2, timeMinimum=1)
            if self.skipFirmwareCheck or self.checkFirmware(self.serialdevice):
                self.is_connected = True
                self.NumberRetryReconnect = 0
                return True
            else:
                False

        except Exception as e:
            self._logger.debug(f"Trying out port {port} failed: "+str(e))

        return False
    
    def _write(self, serialdevice, payload):
        if type(payload) == dict:
            payload = json.dumps(payload)
        serialdevice.write(payload.encode('utf-8'))
        if self.cmdWriteCallBackFct is not None:
            self.cmdWriteCallBackFct(payload)

            
    def _read(self, serialdevice):
        try:
            mLine = serialdevice.readline()
            if self.cmdReadCallBackFct is not None and mLine!=b'' and mLine != b'\n' :
                self.cmdReadCallBackFct(mLine)  
            return mLine
        except SerialException as e:
            self._logger.error("Failed to read the line in serial: "+str(e))
            return False
    
    def setWriteCallback(self, callback):
        '''
        We can assign a callback function to output any of the 
        commands that are sent to the serial device
        '''
        self.cmdWriteCallBackFct = callback
        
    def setReadCallback(self, callback):    
        '''
        We can assign a callback function to output any of the
        responses that are received from the serial device
        '''
        self.cmdReadCallBackFct = callback
    
    def checkFirmware(self, ser):
        """Check if the firmware is correct
        We do not do that inside the queue processor yet
        """
        path = "/state_get"
        payload = {"task": path}

        # write message to the serial
        self._write(ser, payload)
        ser.write(b'\n')

        #self._write(ser, b'\n')
        
        # iterate a few times in case the debug mode on the ESP32 is turned on and it sends additional lines
        for i in range(500):
            # if we just want to send but not even wait for a response
            mReadline = self._read(ser)
            if self.DEBUG and mReadline != "" and mReadline != "\n" and mReadline != b'' and mReadline != b'\n': 
                self._logger.debug("[checkFirmware]: "+str(mReadline))
            if mReadline.decode('utf-8').strip() == "++" or mReadline.decode('utf-8').strip().find("++")>=0:
                self._freeSerialBuffer(ser)
                return True
        return False

    def _generate_identifier(self):
        self.identifier_counter += 1
        return self.identifier_counter

    def _enqueue_command(self, json_command):
        """Add a command to the queue."""
        self.sender_queue.put(json_command)

    def _sending_commands(self):
        """Sending commands from the queue with a 1s delay between them."""
        while self.running:
            
            # Wait for the next command
            json_command = self.sender_queue.get()
            
            # Process the command
            with self.serialLock:
                try:
                    if self.DEBUG and json_command!="": self._logger.debug("[SendingCommands]:"+str(json_command))
                    self._write(self.serialdevice, json.loads(json_command))
                except Exception as e:
                    self._logger.error("Failed to write the line in serial: "+str(e))
            
            # Signal that the command has been processed
            self.sender_queue.task_done()
            if self.DEBUG: self._logger.debug("[SendingCommands]: Task done")
                        
            # Wait for .05 second before processing the next command
            time.sleep(0.05)
            
    def _process_commands(self):
        buffer = ""
        reading_json = False
        currentIdentifier = None
        nLineCountTimeout = 50 # maximum number of lines read before timeout
        lineCounter = 0
        nFailedCommands = 0
        nFailedCommandsMax = 10
        while self.running:
            if self.manufacturer == "UC2Mock": 
                self.running = False
                return

            # device not ready yet
            if self.serialdevice is None:
                self.is_connected = False
                continue
            else:
                self.is_connected = True

            # if we just want to send but not even wait for a response
            with self.serialLock:
                try:
                    mReadline = self._read(self.serialdevice)
                    if mReadline == False :
                        nFailedCommands += 1
                        if nFailedCommands > nFailedCommandsMax:
                            raise Exception("Failed to read the line in serial: "+str(mReadline))
                    line = mReadline.decode('utf-8').strip()
                    if self.DEBUG and line!="": 
                        self._logger.debug("[ProcessLines]:"+str(line))
                except Exception as e:
                    self._logger.error("Failed to read the line in serial: "+str(e))
                    nFailedCommands += 1
                    line = ""
                        
                    # if we have a problem with the serial connection, we need to reconnect
                    for i in range(4):
                        nFailedCommands=0
                        if self.reconnect():
                            self._logger.debug("Reconnected to the serial device")
                            break
                        else:
                            self._logger.debug("Failed to reconnect to the serial device")
                        time.sleep(1)
                
            if line == "++":
                reading_json = True
                continue
            elif line.find("error") != -1:
                # if we have an error, we need to reset the last command
                self._logger.debug("Error - last command did not match the firmware: "+str(self.commands[currentIdentifier]))
                self.resetLastCommand = True
                buffer = ""
                lineCounter = 0
                reading_json = False
                self.responses[currentIdentifier].append({"error": 1})
                self.responses[currentIdentifier].append({"qid": currentIdentifier})
            elif line == "--" or lineCounter>nLineCountTimeout:
                lineCounter = 0
                reading_json = False
                try:
                    json_response = json.loads(buffer)
                    self._logger.debug("[ProcessCommands]: "+str(json_response))
                    if len(self.callBackList) > 0:
                        for callback in self.callBackList:
                            # check if json has key
                            try:
                                if callback["pattern"] in json_response:
                                    callback["callbackfct"](json_response)
                            except Exception as e:
                                self._logger.error("[ProcessCommands Casting Callbacks]: "+str(e))

                except Exception as e:
                    self._logger.error("Failed to load the json from serial %s" % buffer)
                    self._logger.error("Error: %s" % str(e))
                    json_response = {}
                    #reading_json = True

                try: currentIdentifier = json_response["qid"]
                except: pass

                with self.lock:
                    try:
                        self.responses[currentIdentifier].append(json_response.copy())
                    except:
                        self.responses[currentIdentifier] = list()
                        self.responses[currentIdentifier].append(json_response.copy())
                    buffer = ""      # reset buffer

            # detect a reboot of the device and return the current QIDs
            elif line == "reboot":
                self._logger.warning("Device rebooted")
                self.resetLastCommand = True
                buffer = ""
                lineCounter = 0
                reading_json = False
                self.responses[currentIdentifier].append({"reboot": 1})
                self.responses[currentIdentifier].append({"qid": currentIdentifier})

            if reading_json:
                buffer += line
                lineCounter +=1
        self.running = False

    def get_json(self, path, timeout=1):
        message = {"task":path}
        message = json.dumps(message)
        return self.sendMessage(message, nResponses=0, timeout=timeout)

    def post_json(self, path, payload, getReturn=True, nResponses=1, timeout=100):
        """Make an HTTP POST request and return the JSON response"""
        if payload is None:
            payload = {}
        if "task" not in payload:
            payload["task"] = path

        # write message to the serial
        if not getReturn:
            nResponses = -1
        writeResult = self.sendMessage(command=payload, nResponses=nResponses, timeout=timeout)
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
        if type(command) == str:
            command = json.loads(command)
        identifier = self._generate_identifier()
        command["qid"] = identifier
        try:
            json_command = json.dumps(command)+"\n"
            #self.serialdevice.flush()
            if self.DEBUG and json_command!="": self._logger.debug("[SendingCommands]:"+str(json_command))
            with self.serialLock:
                self._write(self.serialdevice, json_command)
            #time.sleep(1)
            # we have to queue the commands and give it some time to process
            #self._enqueue_command(json_command)
            
        except Exception as e:
            if self.DEBUG: self._logger.error(e)
            return "Failed to Send"
        self.commands[identifier]=command # FIXME: Need to clear this after the response is received
        if nResponses <= 0 or not self.is_connected or self.manufacturer=="UC2Mock":
            time.sleep(0.1)
            return identifier
        t0 = time.time()
        maxRetry = 3
        iRetry = 0
        while self.running:
            time.sleep(0.002)
            if self.resetLastCommand or time.time()-t0>timeout or not self.is_connected:
                self.resetLastCommand = False
                return "communication interrupted by timeout or reset: "+str(identifier) + " and code:"+str(self.commands[identifier])
            with self.lock:
                if identifier in self.responses:
                    if len(self.responses[identifier])==nResponses:
                        returnMessage = self.responses[identifier]
                        # remove the response from the list
                        del self.responses[identifier]
                        return returnMessage
                if -identifier in self.responses:
                    self._logger.debug("You have sent the wrong command!")
                    return "Wrong Command"
            if time.time()-t0>self.timeReturnReceived and not (identifier in self.responses and len(self.responses[identifier]) > 0):
                if iRetry > maxRetry:
                    self.resetLastCommand = True
                    return "No response received"
                self._logger.debug("It takes too long to get a response, we will resend the last command: "+str(self.commands[identifier]))
                try:
                    ERROR="We have a queue, so after a while we need to resend the wrong command!"
                    iRetry += 1
                    raise Exception(ERROR)
                    self.serialdevice.write(json.dumps(self.commands[identifier]).encode('utf-8'))
                    t0 = time.time()
                    time.sleep(0.1)
                except Exception as e:
                    self._logger.error("Failed to write the line in serial: "+str(e))
            

    def interruptCurrentSerialCommunication(self):
        self.resetLastCommand = True

    def stop(self):
        self.running = False
        self.thread.join()
        self.serialdevice.close()

    def closeSerial(self):
        self.stop()
        self.running = False

    def close(self):
        self.closeSerial()
        
    def reconnect(self, baudrate=None):
        self._logger.debug("Reconnecting to the serial device")
        self.running = False
        if baudrate is not None:
            self.baudrate = baudrate
        try:
            self.serialdevice.close()
        except:
            pass
        self.serialdevice = self.openDevice(port = self.serialport, baud_rate = self.baudrate)
        if self.serialdevice: return True
        return False


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

    def flush(self):
        pass

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
        while self.is_open:
            if random.random() < 0.2:  # Simulate occasional data availability
                self.data_buffer.extend([random.randint(0, 255) for _ in range(10)])
            time.sleep(0.1)

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()


if __name__ == "__main__":
    # Usage example
    monitor = Serial('/dev/cu.SLAB_USBtoUART', baudrate=128000, DEBUG=True)# 115200)  # Change to your port

    command_to_send = {
            "task": "/state_get"
    }

    command_to_send = {"task":"/motor_act","qid":0,"motor":{"steppers": [{ "stepperid": 3, "position": -1000, "speed": 15000, "isabs": 0, "isaccel":0}]}}

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