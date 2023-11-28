#%%
import uc2rest
import numpy as np
import time

port = "unknown"
port = "/dev/cu.SLAB_USBtoUART"
port = "COM3"
ESP32 = uc2rest.UC2Client(serialport=port, baudrate=500000, DEBUG=True)
ESP32.motor.move_x(steps=10000, speed=10000, is_blocking=True)


#%%
''' ################
HOME
################'''
ESP32.home.home_x(speed =15000, direction = -1, endposrelease = 3000, timeout=2, isBlocking=True)
ESP32.home.home_y(speed =15000, direction = 1, endposrelease = 3000, timeout=2, isBlocking=True)

# setting debug output of the serial to true - all message will be printed
ESP32.serial.DEBUG=True
ESP32.motor.move_x(steps=10000, speed=10000, is_blocking=False)
ESP32.motor.move_a(steps=10000, speed=10000, is_blocking=False)
ESP32.motor.move_z(steps=10000, speed=10000, is_blocking=True)
ESP32.motor.move_x(steps=-10000, speed=10000, is_blocking=True)
ESP32.motor.move_x(steps=10000, speed=10000, is_blocking=False)
mState = ESP32.state.get_state()


''' ################
MODULES
################'''
#load modules from pyhton
mModules = ESP32.modules.get_default_modules()
assert mModules["home"] == 0 or mModules["home"] == 1, "Failed loading the default modules"
print(mModules) #{'led': True, 'motor': True, 'home': True, 'analogin': False, 'pid': False, 'laser': True, 'dac': False, 'analogout': False, 'digitalout': False, 'digitalin': True, 'scanner': False, 'joy': False}

# load modules from device
mModulesDevice = ESP32.modules.get_modules()
#assert mModulesDevice["home"] == 0 or mModulesDevice["home"] == 1, "Failed loading the modules from the device"
print(mModulesDevice) #{'led': True, 'motor': True, 'home': True, 'analogin': False, 'pid': False, 'laser': True, 'dac': False, 'analogout': False, 'digitalout': False, 'digitalin': True, 'scanner': False, 'joy': False}
mModules['home']=1 # activate home module
#%%


ESP32.motor.move_x(steps=10000, speed=10000, is_blocking=True)

# {"task":"/ledarr_act", "led":{"LEDArrMode":1, "led_array":[{"id":0, "r":255, "g":255, "b":255}]}}
mResult = ESP32.led.send_LEDMatrix_full(intensity=(255, 255, 255))
mResult = ESP32.led.send_LEDMatrix_full(intensity=(0, 0, 0), getReturn=False)


# check if we are connected 
# see if it's the right device
mState = ESP32.state.get_state()
assert mState["identifier_name"] == "UC2_Feather", "Wrong device connected"

#%% 
# test Motor
if 0:
    ESP32.rotator.move_x(steps=10000, speed=10000, is_blocking=True)
    ESP32.rotator.move_y(steps=1000, speed=1000, is_blocking=True, is_enabled=False)
    ESP32.rotator.move_z(steps=1000, speed=1000, is_blocking=True)
    ESP32.rotator.move_y(steps=1000, speed=1000, is_blocking=True, is_enabled=False)
    ESP32.rotator.move_t(steps=1000, speed=1000)
    ESP32.rotator.move_xyzt(steps=(0,1000,100,0), speed=10000, is_blocking=True)
    ESP32.rotator.move_xyzt(steps=(0,0,0,0), speed=10000, is_absolute=True, is_blocking=True)
    ESP32.motor.move_forever(speed=(0,100,0,0), is_stop=False)
    ESP32.motor.move_forever(speed=(0,100,0,0), is_stop=False)
    time.sleep(1)


''' ################
SERIAL
################'''
# Write a message, get the id and read it out of the message buffer
test_cmd = "{'task': '/motor_get'}"
test_cmd = '{"task":"/ledarr_act", "led":{"LEDArrMode":1, "led_array":[{"id":0, "r":255, "g":255, "b":255}]}}'
qID = ESP32.serial.writeSerial(test_cmd)
cmd_return = ESP32.serial.readSerial(qID, 1)
print(cmd_return)

''' ################
Digital out
################'''
if(0):
    ESP32.digitalout.setup_digitaloutpin(id=1, pin=4)
    ESP32.digitalout.setup_digitaloutpin(id=2, pin=0)
    ESP32.digitalout.set_trigger(trigger1=True, delayOn1=10, delayOff1=10, trigger2=True, delayOn2=100, delayOff2=10, trigger3=False, delayOn3=0, delayOff3=0)
    time.sleep(1)
    ESP32.digitalout.reset_triggertable()





''' ################
LED 
################'''
# test LED
mResult = ESP32.led.send_LEDMatrix_full(intensity=(255, 255, 255))
assert mResult["success"] == 1, "Failed sending LED command"
time.sleep(0.5)
mResult = ESP32.led.send_LEDMatrix_full(intensity=(0, 0, 0))
assert mResult["success"] == 1, "Failed sending LED command"

# single LED
ESP32.setDebugging(False)
for iLED in range(5):
    # timeout = 0 means no timeout => mResult will be rubish!
    mResult = ESP32.led.send_LEDMatrix_single(indexled=iLED, intensity=(255, 255, 255), timeout=0.)
    mResult = ESP32.led.send_LEDMatrix_single(indexled=iLED, intensity=(0, 0, 0), timeout=0.)

# display random pattern
for i in range(5):
    led_pattern = np.random.randint(0,55, (25,3))
    mResult = ESP32.led.send_LEDMatrix_array(led_pattern=led_pattern,timeout=0)
    assert mResult["success"] == 1, "Failed sending LED command"

time.sleep(3)
#%% left
if(0):
    led_pattern = np.zeros((25,3))
    list_left = (0,1,2,3,4,5,9,10,11,12,13,14,15,16,17)
    list_right = (0,5,6,7,8,9,18,19,20,21,22,23,24)
    led_pattern[list_left,0] = 255
    led_pattern[list_right,1] = 255
    ESP32.led.send_LEDMatrix_array(led_pattern=led_pattern, timeout=1)
    time.sleep(1)
    ESP32.led.send_LEDMatrix_array(led_pattern=led_pattern*0, timeout=1)

#%%
#%%
''' ################
MOTOR
################'''
ESP32.setDebugging(True)

# mResult = ESP32.motor.move_x(steps=0, is_enabled=False)
mResult = ESP32.motor.set_motor_enable(enable=1)
mResult = ESP32.motor.set_motor_enable(enable=0)
assert mResult["success"] == 1, "Failed sending motor command"

# test Motor
mResult = ESP32.motor.set_position(axis=0, position=1000)
assert mResult["success"] == 1, "Failed sending motor command"

position1 = ESP32.motor.get_position(timeout=1)
assert position1[0]==1000, "Failed getting motor position"
print(position1)
ESP32.motor.set_motor_enable(enable=1, enableauto=False)# always on
ESP32.motor.move_x(steps=10000, speed=10000, is_blocking=True)
ESP32.motor.move_y(steps=1000, speed=1000, is_blocking=True, is_enabled=False)
ESP32.motor.move_z(steps=1000, speed=1000, is_blocking=True)
ESP32.motor.move_y(steps=1000, speed=1000, is_blocking=True, is_enabled=False)
ESP32.motor.move_a(steps=1000, speed=1000)
ESP32.motor.move_xyza(steps=(0,1000,100,0), speed=10000, is_blocking=True)
ESP32.motor.move_xyza(steps=(0,0,0,0), speed=10000, is_absolute=True, is_blocking=True)
ESP32.motor.move_forever(speed=(0,100,0,0), is_stop=False)
ESP32.motor.move_forever(speed=(0,100,0,0), is_stop=False)
time.sleep(1)
ESP32.motor.move_forever(speed=(0,0,0,0), is_stop=True)

position2 = ESP32.motor.get_position(timeout=1)
print(position2)

dDist = 1000
speed = 20000
nDist = 4

# test Motor in scanning mode
ESP32.motor.move_xyza(steps=(0,0,0,0), speed=speed, is_absolute = True, is_blocking=True)

for ix in range(nDist):
    for iy in range(nDist):
        if ix%2==0:
            iy=nDist-iy
        ESP32.motor.move_xyza(steps=(0,ix*dDist,iy*dDist,0), speed=speed, is_absolute = True, is_blocking=True)
ESP32.motor.move_xyza(steps=(0,nDist*dDist,nDist*dDist,0), speed=speed, is_absolute = True, is_blocking=True)
ESP32.motor.move_xyza(steps=(0,0,0,0), speed=speed, is_absolute = True, is_blocking=True)

#%%

''' ################
analog
################'''
ESP32.analog.set_analog(readanaloginID=1, readanaloginPIN=35, nanaloginavg=1)
ESP32.analog.get_analog(readanaloginID=1)
#analogValueAVG = ESP32.analog.read_sensor(sensorID=1, NAvg=100)
#print(analogValueAVG)


''' ################
LASER 
################'''
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
ESP32.wifi.scanWifi()
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
ESP32.state.pairBT(1)
ESP32.state.setControllerMode(isController=True)


_busy = ESP32.state.isBusy()
print(_busy)
