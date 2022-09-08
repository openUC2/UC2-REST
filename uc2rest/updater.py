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
            

        
        if firmwareDownloadPath is None:
            self.firmwareDownloadPath = "https://raw.githubusercontent.com/openUC2/UC2-REST/master/ESP32/main/build/esp32.esp32.esp32/"
        else:   
            self.firmwareDownloadPath = firmwareDownloadPath


        # all necessary binaries for flashingthe firmware
        self.filenames = []
        self.filenames.append("main.ino.bin")
        self.filenames.append("main.ino.bootloader.bin")
        self.filenames.append("main.ino.map")
        self.filenames.append("main.ino.partitions.bin")
        self.filenames.append("boot_app0.bin")

            
    def flashFirmware(self, firmwarePath=None):
        # sideload the firmware if already available online
        if firmwarePath is not None:
            return
        try:
            #esptool.py --chip esp32 --port /dev/cu.SLAB_USBtoUART --baud 921600 
            # --before default_reset --after hard_reset write_flash -z --flash_mode dio 
            # --flash_freq 80m --flash_size 4MB 
            # 0x1000 ./ESP32/build/main.ino.bootloader.bin 
            # 0x8000 ./ESP32/build/main.ino.partitions.bin 
            # 0xe000 ./ESP32/build/boot_app0.bin 
            # 0x10000 ./ESP32/build/main.ino.bin 
            try:
                cmd = ["esptool.py", 
                        "--chip", "esp32",
                        "--port", self.port, 
                        "--baud", "921600", 
                        "write_flash",
                        "--flash_freq", "80m",
                        "--flash_mode", "dio", 
                        "--flash_size", "detect", 
                        "0xe000", self.filenames[4],
                        "0x1000", self.filenames[1], 
                        "0x8000", self.filenames[3], 
                        "0x10000", self.filenames[0]]
                print('Using command %s' % ' '.join(cmd))
                process = subprocess.Popen(cmd)
                process.wait()
            except Exception as e:
                print(e)
                print("We will try an alternative route:")
                cmd = ["python -m esptool", 
                        "--chip", "esp32",
                        "--port", self.port, 
                        "--baud", "921600", 
                        "write_flash",
                        "--flash_freq", "80m",
                        "--flash_mode", "dio", 
                        "--flash_size", "detect", 
                        "0xe000", self.filenames[4],
                        "0x1000", self.filenames[1], 
                        "0x8000", self.filenames[3], 
                        "0x10000", self.filenames[0]]
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
        # download the firmware from github
        fileCounter = 0
        for filename in self.filenames:
            response = requests.get(self.firmwareDownloadPath+filename)
            # check if folder exists and create it if not
            if response.status_code == 200:
                
                try:
                    if not os.path.exists(self.firmwarePath):
                        os.makedirs(self.firmwarePath)
                    # remove file if exists
                    if os.path.exists(self.firmwarePath+filename):
                        os.remove(self.firmwarePath+filename)    
                    open(self.firmwarePath+filename, "wb").write(response.content) 
                    print("Succesfully downloaded file: "+self.firmwarePath+filename)
                    fileCounter+=1
                except Exception as e:
                    print(e)
                    print("Firmware download failed"+filename)

        if fileCounter == len(self.filenames):
            return True
        else:
            return False
                    
    def removeFirmware(self):
        for filename in self.filenames:
            try:
                print("Removing Firmware:"+filename)
                os.remove(filename)
            except Exception as e:
                print(e)
        return True
        
if __name__ == "__main__":
    updater = updater(port="/dev/cu.SLAB_USBtoUART", firmwarePath=None)
    updater.downloadFirmware()
    updater.flashFirmware()
    
    # remove firmware.bin after flashing
    updater.removeFirmware()
    
            
        
                      