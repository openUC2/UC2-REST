import uc2rest as uc2

# define the serial port
serialport = "/dev/cu.SLAB_USBtoUART"
serialport = "COM3"

# optional: create an ESP32 objejct
# ESP32 = uc2.UC2Client(serialport=serialport)

# create the updater object
updater = uc2.updater(port=serialport)

updater.downloadFirmware()
updater.flashFirmware()

# remove firmware.bin after flashing
updater.removeFirmware()
