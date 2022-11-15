import uc2rest as uc2
import time 

# create the 
ESP32 = uc2.UC2Client(serialport="/dev/cu.SLAB_USBtoUART")
ESP32.serial.DEBUG=True

# see if it's the right device
ESP32.state.get_state()

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

