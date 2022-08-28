import esptool
import requests
import subprocess
import os 
class updater(object):
    
    def __init__(self, port=None, firmwarePath="./", firmwareDownloadPath=None):
        self.port = port
        self.firmwarePath = firmwarePath
        self.FWfileName = 'firmware.bin'
        
        if firmwareDownloadPath is None:
            self.firmwareDownloadPath = "https://raw.githubusercontent.com/openUC2/UC2-REST/master/ESP32/build/main.ino.bin"
        else:   
            self.firmwareDownloadPath = firmwareDownloadPath
            
    def flashFirmware(self):
        try:
            cmd = ["esptool.py", 
                    "--chip", "esp32",
                    "--port", self.port, 
                    "--baud", "921600", 
                    "write_flash",
                    "--flash_mode", "dio", 
                    "--flash_size", "detect", 
                    "0x0", self.FWfileName]
            print('Using command %s' % ' '.join(cmd))
            process = subprocess.Popen(cmd)
            process.wait()
            print("Firmware flashed")
            return True
        except Exception as e:
            print(e)
            print("Firmware flash failed")
            return False
    
    def downloadFirmware(self):
        print("Downloading Firmware")
        response = requests.get(self.firmwareDownloadPath)
        open(self.FWfileName, "wb").write(response.content)
        
    def removeFirmware(self):
        try:
            print("Removing Firmware")
            os.remove(self.FWfileName)
            return True
        except Exception as e:
            print(e)
            return False
            
if __name__ == "__main__":
    updater = updater(port="/dev/cu.SLAB_USBtoUART", firmwarePath=None)
    updater.downloadFirmware()
    updater.flashFirmware()
    
    # remove firmware.bin after flashing
    updater.removeFirmware()
    
            
        
                      