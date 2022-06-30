
import serial
import time
import json 

port = "/dev/cu.SLAB_USBtoUART"
port = "/dev/cu.wchusbserial1430"
arduino1 = serial.Serial(port=port, baudrate=115200, timeout=1)


def writeSerial(Text):
    Text = json.dumps(Text)
    arduino1.flushInput()
    arduino1.flushOutput()    
    arduino1.write(Text.encode(encoding='UTF-8'))
    return Text

def readSerial():   
    Text = ''
    rmessage = ' ' 
    iiter = 0
    while True: 
        rmessage = arduino1.readline().decode()
        Text += rmessage
        if rmessage.find("//")==0: break

    return Text

def doSomething(i):
    Text = {"Geschwindigkeit": [1000, i*12, i], "Richtung":[1,1,-1]}
    return Text
        
#%%



Text = {"task": "motor_act",
        "Axis": 0,
        "Speed": 0,
        "Position": 0,
        "isabsolute": 0}

Text = {"task":"/motor_act", "axis":1, "speed": 100, "position": 1000, "isabsolute":1}
Text = {"task":"motor_set", "set_task": "currentposition", "currentposition": 1000, "axis":1}
Text = {"task":"/motor_get", "axis":1}
Text = {"task":"motor_set", "set_task": "stop", "axis":1}

Text = {"task":"/DAC_act", "frequency": 1000, "offset":00, "dac_channel": 2, "amplitude":0}

Text = {"task":"/LASER_act", "LASERid": 1, "LASERval": 1000}


Text = {"task":"/motor_act", "axis":1, "speed": 10, "position": 10, "isblocking":1, "isabsolute":1}


print("send: " + writeSerial(Text))                
print(str(readSerial()))
'''
Values = readSerial()
if Values != False:
    print(Values["Geschwindigkeit"])
    print(Values["Geschwindigkeit"][-1])
time.sleep(1)
'''
