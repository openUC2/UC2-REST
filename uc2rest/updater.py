import esptool
import requests
import subprocess
import os 
class updater(object):
    
    def __init__(self, ESP32=None, port=None, firmwarePath="./", firmwareDownloadPath=None):
        if ESP32 is not None:
            self.port = ESP32.serialport
        if port is not None:
            self.port = port
            

        # define a temporary firmware file name for the firmware download
        if firmwarePath is not None:
            self.firmwarePath = firmwarePath
        else:
            self.firmwarePath = "./"
            
        self.FWfileName = 'firmware.bin'
        
        if firmwareDownloadPath is None:
            self.firmwareDownloadPath = "https://raw.githubusercontent.com/openUC2/UC2-REST/master/ESP32/build/main.ino.bin"
        else:   
            self.firmwareDownloadPath = firmwareDownloadPath
            
    def flashFirmware(self, firmwarePath=None):
        # sideload the firmware if already available online
        if firmwarePath is not None:
            self.FWfileName = firmwarePath
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
        # check if folder exists and create it if not
        if response.status_code == 200:
            
            try:
                if not os.path.exists(self.firmwarePath):
                    os.makedirs(self.firmwarePath)
                open(self.firmwarePath+self.FWfileName, "wb").write(response.content) 
                return True
            except Exception as e:
                print(e)
                print("Firmware download failed")
                return False
        else:
            return False
                
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
    
            
        
                      