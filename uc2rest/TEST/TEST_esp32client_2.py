import uc2rest
import numpy as np
import time

port = "unknown"
port = "/dev/cu.SLAB_USBtoUART"
port = "/dev/cu.wchusbserial14310"
#port = "/dev/cu.wchusbserial1440"
port = "/dev/cu.wchusbserial110"

ESP32 = uc2rest.UC2Client(serialport=port)
# setting debug output of the serial to true - all message will be printed
ESP32.serial.DEBUG=True


ESP32.motor.set_motor_enable(axis=0, is_enable=0)

''' ################
SERIAL
################'''
test_cmd = "{'task': '/motor_get'}"
ESP32.serial.writeSerial(test_cmd)
cmd_return = ESP32.serial.readSerial()
print(cmd_return)

''' ################
Digital out
################'''
ESP32.digitalout.setup_digitaloutpin(id=1, pin=4)
ESP32.digitalout.setup_digitaloutpin(id=2, pin=0)
ESP32.digitalout.set_trigger(trigger1=True, delayOn1=10, delayOff1=10, trigger2=True, delayOn2=100, delayOff2=10, trigger3=False, delayOn3=0, delayOff3=0)
time.sleep(1)
ESP32.digitalout.reset_triggertable()



''' ################
MODULES
################'''
mModules = ESP32.modules.get_default_modules()
print(mModules)
mModulesDevice = ESP32.modules.get_modules()
print(mModulesDevice)
mModules['home']=1 # activate home module
ESP32.modules.set_modules(mModules)
# wait for reboot
time.sleep(2)
mModulesDevice = ESP32.modules.get_modules()
print(mModulesDevice)


''' ################
HOME
################'''
ESP32.home.home_x(speed =15000, direction = -1, endposrelease = 3000, timeout=20, isBlocking=True)
ESP32.home.home_y(speed =15000, direction = 1, endposrelease = 3000, timeout=20, isBlocking=True)



''' ################
LED 
################'''
# setup led configuration
if ESP32.APIVersion == 2:
    ESP32.led.set_ledpin(ledArrPin=4, ledArrNum=16)
    print(ESP32.led.get_ledpin())

# test LED
ESP32.led.send_LEDMatrix_full(intensity=(255, 255, 255))
time.sleep(0.5)
ESP32.led.send_LEDMatrix_full(intensity=(0, 0, 0))

# display random pattern
for i in range(10):
    led_pattern = np.random.randint(0,55, (25,3))
    ESP32.led.send_LEDMatrix_array(led_pattern=led_pattern,timeout=1)

ESP32.led.send_LEDMatrix_single(indexled=0, intensity=(0, 255, 0))


#%% left
led_pattern = np.zeros((25,3))
list_left = (0,1,2,3,4,5,9,10,11,12,13,14,15,16,17)
list_right = (0,5,6,7,8,9,18,19,20,21,22,23,24)
led_pattern[list_left,0] = 255
led_pattern[list_right,1] = 255
ESP32.led.send_LEDMatrix_array(led_pattern=led_pattern, timeout=1)
time.sleep(1)
ESP32.led.send_LEDMatrix_array(led_pattern=led_pattern*0, timeout=1)

#%%

''' ################
analog
################'''
ESP32.analog.set_analog(readanaloginID=1, readanaloginPIN=35, nanaloginavg=1)
ESP32.analog.get_analog(readanaloginID=1)
#analogValueAVG = ESP32.analog.read_sensor(sensorID=1, NAvg=100)
#print(analogValueAVG)




''' ################
MOTOR
################'''

if 0:
    # sestup all motors at once 
    if ESP32.APIVersion == 1:
        # load the pin config from the ESP32
        mConfig = ESP32.config.loadConfigDevice()
        print(mConfig)

        # reset the dictionary 
        mConfig = dict.fromkeys(mConfig, 0)

        # modify some values
        mConfig["ledArrPin"] = 15
        mConfig["ledArrNum"] = 16

        mConfig["motXstp"] = 2
        mConfig["motXdir"] = 33

        mConfig["motYstp"] = 27
        mConfig["motYdir"] = 16

        mConfig["motZstp"] = 12
        mConfig["motZdir"] = 14

        mConfig["identifier"] = "UniEindhoven"
        mConfig["ssid"] = "UC2"
        mConfig["PW"] = "UC2"

        # now load the config to the ESP32
        ESP32.config.setConfigDevice(mConfig)
        ESP32.config.applyConfigDevice()

        # wait until the ESP reboots and identify the new config
        time.sleep(5)

        # see if it's the right device
        ESP32.state.get_state()
        mConfig = ESP32.config.loadConfigDevice()
        print(mConfig)
        
    if ESP32.APIVersion == 2:
        print(ESP32.motor.settingsdict)
        ESP32.motor.settingsdict["motor"]["steppers"][1]["dir"]=16
        ESP32.motor.settingsdict["motor"]["steppers"][1]["step"]=26
        ESP32.motor.settingsdict["motor"]["steppers"][2]["dir"]=27
        ESP32.motor.settingsdict["motor"]["steppers"][2]["step"]=25
        ESP32.motor.settingsdict["motor"]["steppers"][3]["dir"]=14
        ESP32.motor.settingsdict["motor"]["steppers"][3]["step"]=17
        ESP32.motor.settingsdict["motor"]["steppers"][0]["dir"]=18
        ESP32.motor.settingsdict["motor"]["steppers"][0]["step"]=19
        ESP32.motor.settingsdict["motor"]["steppers"][0]["enable"]=12
        ESP32.motor.settingsdict["motor"]["steppers"][1]["enable"]=12
        ESP32.motor.settingsdict["motor"]["steppers"][2]["enable"]=12
        ESP32.motor.settingsdict["motor"]["steppers"][3]["enable"]=12
        ESP32.motor.set_motors(ESP32.motor.settingsdict)

        # print settings 
        print(ESP32.motor.get_motors())

        # OR setup motors individually (according to WEMOS R32 D1)
        ESP32.motor.set_motor(stepperid = 1, position = 0, stepPin = 26, dirPin=16, enablePin=12, maxPos=None, minPos=None, acceleration=None, isEnable=1)
        ESP32.motor.set_motor(stepperid = 2, position = 0, stepPin = 25, dirPin=27, enablePin=12, maxPos=None, minPos=None, acceleration=None, isEnable=1)
        ESP32.motor.set_motor(stepperid = 3, position = 0, stepPin = 17, dirPin=14, enablePin=12, maxPos=None, minPos=None, acceleration=None, isEnable=1)
        ESP32.motor.set_motor(stepperid = 0, position = 0, stepPin = 19, dirPin=18, enablePin=12, maxPos=None, minPos=None, acceleration=None, isEnable=1)

        # get individual motors
        print(ESP32.motor.get_motor(axis = 1))

        ESP32.motor.set_motor_currentPosition(axis=0, currentPosition=10000)
        ESP32.motor.set_motor_acceleration(axis=0, acceleration=10000)
        ESP32.motor.set_motor_enable(is_enable=1)
        ESP32.motor.set_direction(axis=1, sign=1, timeout=1)
        ESP32.motor.set_position(axis=1, position=0, timeout=1)

        # wait to settle
        time.sleep(2)

# test Motor
position1 = ESP32.motor.get_position(timeout=1)
print(position1)
ESP32.motor.move_x(steps=10000, speed=10000, is_blocking=True)
ESP32.motor.move_y(steps=1000, speed=1000, is_blocking=True)
ESP32.motor.move_z(steps=1000, speed=1000, is_blocking=True)
ESP32.motor.move_t(steps=1000, speed=1000)
ESP32.motor.move_xyzt(steps=(0,10000,10000,0), speed=10000, is_blocking=True)
ESP32.motor.move_xyzt(steps=(0,0,0,0), speed=10000, is_absolute=True, is_blocking=True)
ESP32.motor.move_forever(speed=(0,100,0,0), is_stop=False)
time.sleep(1)
ESP32.motor.move_forever(speed=(0,0,0,0), is_stop=True)

position2 = ESP32.motor.get_position(timeout=1)
print(position2)

dDist = 1000
speed = 20000
nDist = 4

# test Motor in scanning mode
ESP32.motor.move_xyzt(steps=(0,0,0,0), speed=speed, is_absolute = True, is_blocking=True)

for ix in range(nDist):
    for iy in range(nDist):
        if ix%2==0:
            iy=nDist-iy
        ESP32.motor.move_xyzt(steps=(0,ix*dDist,iy*dDist,0), speed=speed, is_absolute = True, is_blocking=True)
ESP32.motor.move_xyzt(steps=(0,nDist*dDist,nDist*dDist,0), speed=speed, is_absolute = True, is_blocking=True)
ESP32.motor.move_xyzt(steps=(0,0,0,0), speed=speed, is_absolute = True, is_blocking=True)

''' ################
LASER 
################'''
# set laser pins 
if ESP32.APIVersion == 2:
    ESP32.laser.set_laserpin(laserid=1, laserpin=15)
    ESP32.laser.set_laserpin(laserid=2, laserpin=16)
    ESP32.laser.set_laserpin(laserid=3, laserpin=17)

# get laser pins
ESP32.laser.get_laserpins()
ESP32.laser.get_laserpin(laserid=1)

# set laser values
ESP32.laser.set_laser(channel=1, value=1000, despeckleAmplitude=0, despecklePeriod=10, timeout=20, is_blocking = True)
ESP32.laser.set_laser(channel=2, value=1000, despeckleAmplitude=0, despecklePeriod=10, timeout=20, is_blocking = True)
ESP32.laser.set_laser(channel=3, value=1000, despeckleAmplitude=0, despecklePeriod=10, timeout=20, is_blocking = True)




''' ################
Wifi
################'''
# wifi
if ESP32.APIVersion == 2:
    ESP32.wifi.scanWifi()


''' ################
Config Manager 
################'''
if ESP32.APIVersion == 2:
    configfile = ESP32.config.loadConfigDevice(timeout=1)
    print(configfile)
    configfile["motorconfig"][0]["dir"]=17 # change parameter
    ESP32.config.setConfigDevice(configfile)

''' ################
State
################'''
# test state
_state = ESP32.state.get_state()
print(_state)
ESP32.state.set_state(debug=False)
_mode = ESP32.state.isControllerMode()
print(_mode)
ESP32.state.espRestart()
time.sleep(5)
ESP32.state.setControllerMode(isController=True)
_busy = ESP32.state.isBusy()
print(_busy)
