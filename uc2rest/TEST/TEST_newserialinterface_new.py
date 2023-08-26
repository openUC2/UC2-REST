import serial
import json
import queue
import threading
import time

class SerialMonitor:
    def __init__(self, port, baud_rate=115200, timeout=5):
        self.ser = self.openDevice(port, baud_rate, timeout)
                
        self.command_queue = queue.Queue()
        self.responses = {}
        self.lock = threading.Lock()
        self.running = True
        self.identifier_counter = 0 # Counter for generating unique identifiers
        self.thread = threading.Thread(target=self._process_commands)
        self.thread.start()

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
                raise Exception("Timeout waiting for device to be ready")
            
    def openDevice(self, port, baud_rate, timeout=5):
        ser = serial.Serial(port, baud_rate, timeout=1)
        ser.write_timeout = 1    
        # need to let device warm up and flush out any old data
        self._freeSerialBuffer(ser)
        return ser 

    def _generate_identifier(self):
        self.identifier_counter += 1
        return f"id_{self.identifier_counter}"

    def _process_commands(self):
        buffer = ""
        reading_json = False
        currentIdentifier = None
        waitForResponse = False
        nResponses = 0
        t0 = 0
        readTimeout = 1
        while self.running:
            if not self.command_queue.empty():
                currentIdentifier, command, readTimeout, nResponses = self.command_queue.get()
                t0 = time.time()
                json_command = json.dumps(command)
                self.ser.write(json_command.encode('utf-8'))
                self.ser.write(b'\n')

            # if we just want to send but not even wait for a response
            if nResponses==0:
                with self.lock:
                    self.responses[currentIdentifier] = ""
                self._freeSerialBuffer(self.ser)
                
            mReadline = self.ser.readline()
            try:
                line = mReadline.decode('utf-8').strip()
                print (line)
            except:
                line = ""
            if line == "++":
                reading_json = True
                continue
            elif line == "--" :
                reading_json = False
                json_response = json.loads(buffer)
                
                nResponses -= 1
                if nResponses == 0:
                    with self.lock:
                        self.responses[currentIdentifier] = json_response
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

    def sendMessage(self, command, readTimeout=1, nResponses=1):
        '''
        Sends a command to the device and optionally waits for a response.
        If nResponses is 0, then the command is sent but no response is expected.
        If nResponses is 1, then the command is sent and the response is returned.
        If nResponses is >1, then the command is sent and a list of responses is returned.        
        '''
        identifier = self._generate_identifier()
        self.command_queue.put((identifier, command, readTimeout, nResponses))

        if nResponses == 0: return -1
        else: return identifier


    def waitForMessage(self, identifier):
        
        while self.running:
            with self.lock:
                if identifier in self.responses:
                    return self.responses.pop(identifier)
            time.sleep(0.001)
            
        with self.lock:
            event_data = self.responses.get(identifier, None)
            
        return event_data
        
    def postJson(self, command, readTimeout=1, nResponses=1):
        identifier = self.sendMessage(command, readTimeout=1, nResponses=1)
        
        if identifier == -1: return ""
        response = self.waitForMessage(identifier)
        return response
    
    def stop(self):
        self.running = False
        self.thread.join()
        self.ser.close()

# Usage example
monitor = SerialMonitor('/dev/cu.SLAB_USBtoUART', baud_rate=115200)  # Change to your port

command_to_send = {
        "task": "/state_get"
}

command_to_send = {"task":"/motor_act","motor":{"steppers": [{ "stepperid": 3, "position": -1000, "speed": 15000, "isabs": 0, "isaccel":0}]}}
    
t0 = time.time()
response = monitor.postJson(command_to_send, nResponses=2)
print("Response:", response)
print("time:", time.time()-t0)

t0 = time.time()
response = monitor.postJson(command_to_send, nResponses=0)
print("Response:", response)
print("time:", time.time()-t0)


#response = monitor.waitForResponse(command_id)

if response:
    print("Response:", response)

monitor.stop()
