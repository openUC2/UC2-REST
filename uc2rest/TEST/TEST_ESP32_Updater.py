import uc2rest as uc2

# define the serial port
serialport = "/dev/cu.SLAB_USBtoUART"

# optional: create an ESP32 objejct
#ESP32 = uc2.UC2Client(serialport=serialport)

# create the updater object
updater = uc2.updater(port=serialport, firmwarePath="./")
#updater = uc2.updater(port="COM4", firmwarePath="./")
#updater = uc2.updater(ESP32=ESP32, firmwarePath="./")
updater.downloadFirmware()
updater.flashFirmware()

# remove firmware.bin after flashing
updater.removeFirmware()

