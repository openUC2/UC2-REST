import esptool
import requests

class updater(object):
    
    def __init__(self, port=None, firmwarePath="./", firmwareDownloadPath=None):
        self.port = port
        self.firmwarePath = firmwarePath
        
        if firmwareDownloadPath is None:
            self.firmwareDownloadPath = "https://raw.githubusercontent.com/openUC2/UC2-REST/cleanup/ESP32/build/main.ino.bootloader.bin"
        else:   
            self.firmwareDownloadPath = firmwareDownloadPath
        
            
    def flashFirmware(self):
        try:
            cmd = [
                #'python -m', 'esptool.py',
				'--port', self.port,
                '--chip', 'esp32',
				'--baud','921600',
				'--after', 'no_reset', 'write_flash',
				'--flash_mode', 'dio',
                '--flash_size', '0x0',
                'firmware.bin'
			]
            print('Using command %s' % ' '.join(cmd))
            #import subprocess
            #import sys
            #subprocess.check_call(cmd, shell=True, stdout=sys.stdout, stderr=subprocess.STDOUT)
            esptool.main(cmd)
        except Exception as e:
            print(e)
    
    def downloadFirmware(self):
        response = requests.get(self.firmwareDownloadPath)
        open("firmware.bin", "wb").write(response.content)
        
            
if __name__ == "__main__":
    updater = updater(port="/dev/cu.SLAB_USBtoUART", firmwarePath=None)
    updater.downloadFirmware()
    updater.flashFirmware()
            
        
                      