import json
class cmdRecorder(object):
    '''
    A class for recording and playing back commands sent to a device.

    Attributes:
    -----------
    _parent : object
        The parent object that the cmdRecorder is attached to.
    commandQueue : list
        A list of commands that have been recorded.

    Methods:
    --------
    startRecording()
        Starts recording commands by storing them in the commandQueue.
    stopRecording()
        Stops recording commands and clears the commandQueue.
    addCommand(command)
        Adds an individual hardware command to the commandQueue.
    printCommands()
        Prints the previously recorded commands.
    playCommands(nTimes=1, isBlocking=True)
        Plays the commands in the commandQueue nTimes.
    '''

    def __init__(self, parent):
        '''
        Initializes a new instance of the cmdRecorder class.

        Parameters:
        -----------
        parent : object
            The parent object that the cmdRecorder is attached to.
        '''
        self._parent = parent
        self.commandQueue = []
        
    def startRecording(self):
        '''
        Starts recording commands by storing them in the commandQueue.
        '''
        self.commandQueue = []
        self._parent.serial.toggleCommandOutput(cmdCallBackFct = self.addCommand)
        
    def stopRecording(self):
        '''
        Stops recording commands and clears the commandQueue.
        '''
        self._parent.serial.toggleCommandOutput(cmdCallBackFct = None)

    def addCommand(self, command):
        '''
        Adds an individual hardware command to the commandQueue.

        Parameters:
        -----------
        command : str
            The command to add to the commandQueue.
        '''
        self.commandQueue.append(command)
        
    def printCommands(self):
        '''
        Prints the previously recorded commands.
        '''
        print(self.commandQueue)
        
    def playCommands(self, nTimes=1, isBlocking=False):
        '''
        Plays the commands in the commandQueue nTimes.
        # like so: 
        # {"tasks":[{"task":"/state_get"},{"task":"/state_act", "delay":1000}],"nTimes":2}
        # {'tasks:': '[{"task": "/state_get"}, {"task": "/state_act", "delay": 1000}]', 'nTimes': 3, 'qid': 2}
        Parameters:
        -----------
        nTimes : int, optional
            The number of times to play the commands. Default is 1.
        isBlocking : bool, optional
            Whether or not to wait for the device to respond before sending the next command. Default is True.
        '''
        payload = {"tasks":self.commandQueue, "nTimes":nTimes}
        nResponses = len(self.commandQueue) * nTimes * isBlocking # not quite true since motors will have multiple returns..but fine for now
        r = self._parent.serial.sendMessage(command=payload, nResponses=nResponses)
        return r