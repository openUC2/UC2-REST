import uc2rest as uc2

ESP32 = uc2.ESP32Client(serialport="unknown")

updater = uc2.updater(port="/dev/cu.SLAB_USBtoUART", firmwarePath="./")
#<updater = uc2.updater(ESP32=ESP32, firmwarePath="./")
updater.downloadFirmware()
updater.flashFirmware()

# remove firmware.bin after flashing
updater.removeFirmware()

