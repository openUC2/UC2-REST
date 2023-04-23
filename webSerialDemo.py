from js import navigator
from pyodide.ffi import to_js
from pyodide.ffi.wrappers import add_event_listener
import pkgutil
print("start webserialdemo.py")
from uc2rest.UC2Client import UC2Client
#Utility function for converting py dicts to JS objects
def j(obj):
    return to_js(obj, dict_converter=js.Object.fromEntries)

class SerialManager():
    '''
    Class for managing reads and writes to/from a serial port
    Not very clean! No error handling, no way to stop listening etc.
    '''
    
    def __init__(self) -> None:
        '''
        Create an instance of the SerialManager class when this script runs
        '''
        self.UC2Client = None
        
    async def askForSerial(self):
        '''
        Request that the user select a serial port, and initialize
        the reader/writer streams with it
        '''
        if not hasattr(navigator, 'serial'):
            warning = "This browser does not support WebSerial; see https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API#browser_compatibility for a list of compatible browsers."
            print(warning)
            raise NotImplementedError(warning)
        
        self.port = await navigator.serial.requestPort()
        await self.port.open(j({"baudRate": 115200}))
        js.console.log("OPENED PORT")

        # Set up encoder to write to port
        self.encoder = js.TextEncoderStream.new()
        outputDone = self.encoder.readable.pipeTo(self.port.writable)

        # Set up listening for incoming bytes
        self.decoder = js.TextDecoderStream.new()
        inputDone = self.port.readable.pipeTo(self.decoder.writable)
        inputStream = self.decoder.readable

        self.reader = inputStream.getReader();

        # assign port to UC2Client
        self.UC2Client = UC2Client(SerialManager=self)

        await self.listenAndEcho()
        

        
    async def write(self, data):
        '''Write to the serial port'''
        outputWriter = self.encoder.writable.getWriter()
        outputWriter.write(data + '\n')
        outputWriter.releaseLock()
        js.console.log(f"Wrote to stream: {data}")

    async def listenAndEcho(self):
        '''Loop forever, echoing values received on the serial port to the JS console'''
        receivedValues = []
        while (True):
            response = await self.reader.read()
            value, done = response.value, response.done
            if ('\r' in value or '\n' in value):
                #Output whole line and clear buffer when a newline is received
                print(f"Received from Serial: {''.join(receivedValues)}")
                receivedValues = []
            elif (value):
                #Output individual characters as they come in
                print(f"Received Char: {value}")
                receivedValues.append(value)

    async def read(self):
        """Read from the serial port"""
        receivedValues = []
        response = self.reader.read()
        
        while (True):
            response = self.reader.read()
            value, done = response.value, response.done
            if ('\r' in value or '\n' in value):
                #Output whole line and clear buffer when a newline is received
                print(f"Received from Serial: {''.join(receivedValues)}")
                # receivedValues = []
                return receivedValues
            elif (value):
                #Output individual characters as they come in
                print(f"Received Char: {value}")
                receivedValues.append(value)
        
        
    
#Create an instance of the SerialManager class when this script runs
sm = SerialManager()


async def getState(sm: SerialManager):
    '''Get the state of the serial port'''
    await sm.UC2Client.state.get_state()

#A helper function - to point the py-click attribute of one of our buttons to
async def sendValueFromInputBox(sm: SerialManager):
    '''
    Get the value of the input box and write it to serial
    Also clears the input box
    '''
    textInput = js.document.getElementById("text")
    value = textInput.value
    textInput.value = ''
    print(f"Writing to Serial Port: {value}")

    await sm.write(value)
