import subprocess
import os
import esptool
import tempfile
import urllib
import threading
import zipfile
import progressbar 
import platform
import glob
try:
    import requests
    is_requests = True
except:
    is_requests = False

class updater(object):
    
    def __init__(self, ESP32=None, port=None, parent=None):
        if ESP32 is not None:
            self.port = ESP32.serial.serialport
        if port is not None:
            self.port = port
        # parent holds the entire esp object
        if parent is not None:
            self._parent = parent
            self.port = self._parent.serial.serialport

        # define a temporary firmware file name for the firmware download
        self.firmwarePath = os.path.join(tempfile.gettempdir(), "uc2rest")
            

                
    def unzipFiles(self):
        ''' Unzip the UC2Rest.zip '''
        print("Unzipping files")
        with zipfile.ZipFile(os.path.join(self.firmwarePath, self.uc2restZip), 'r') as zip_ref:
            zip_ref.extractall(self.firmwarePath)
        # list the files in the directory
        filenames = os.listdir(self.firmwarePath)
        print("Done unzipping files:")
        print(filenames)
        return filenames
        
        
    def flashFirmware(self, firmwarePath=None):
        # sideload the firmware if already available online
        
        self.filenames = self.unzipFiles()
        if self._parent is not None:
            # in case the serial is still open, perhaps it makes sense to close it
            try:
                self._parent.serial.closeSerial()
            except:
                pass
            
        if platform.system()=="Windows":
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
                            "0xe000", os.path.join(self.firmwarePath,"boot_app0.bin"),
                            "0x1000", os.path.join(self.firmwarePath,"main.ino.bootloader.bin"), 
                            "0x8000", os.path.join(self.firmwarePath,"main.ino.partitions.bin"), 
                            "0x10000", os.path.join(self.firmwarePath,"main.ino.bin")]
                    print('Using command %s' % ' '.join(cmd))
                    process = subprocess.Popen(cmd,shell=True,stderr=subprocess.PIPE, stdout=subprocess.PIPE)
                    process.wait()
                    stdout, stderr = process.communicate()
                    if str(stderr).find("not reco")>0 or not str(stdout).find("Leaving...")>0: 
                        raise Exception
                    
                except Exception as e:
                    print(e)
                    print("We will try an alternative route:")
                    try:
                        cmd = ["python -m esptool", 
                                "--chip", "esp32",
                                "--port", self.port, 
                                "--baud", "921600", 
                                "write_flash",
                                "--flash_freq", "80m",
                                "--flash_mode", "dio", 
                                "--flash_size", "detect", 
                                "0xe000", os.path.join(self.firmwarePath,"boot_app0.bin"),
                                "0x1000", os.path.join(self.firmwarePath,"main.ino.bootloader.bin"), 
                                "0x8000", os.path.join(self.firmwarePath,"main.ino.partitions.bin"), 
                                "0x10000", os.path.join(self.firmwarePath,"main.ino.bin")]
                        print('Using command %s' % ' '.join(cmd))
                        env = os.environ
                        process = subprocess.Popen(cmd,shell=True, env=env,stderr=subprocess.PIPE, stdout=subprocess.PIPE)
                        process.wait()
                        stdout, stderr = process.communicate()
                        if str(stderr).find("not reco")>0 or  not str(stdout).find("Leaving...")>0: 
                            raise Exception
                    except Exception as e:
                        print(e)
                        print("We will try an alternative route:")
                        try:
                            # special weindows case?
                            cmd = ["esptool", 
                                    "--chip", "esp32",
                                    "--port", self.port, 
                                    "--baud", "921600", 
                                    "write_flash",
                                    "--flash_freq", "80m",
                                    "--flash_mode", "dio", 
                                    "--flash_size", "detect", 
                                    "0xe000", os.path.join(self.firmwarePath,"boot_app0.bin"),
                                    "0x1000", os.path.join(self.firmwarePath,"main.ino.bootloader.bin"), 
                                    "0x8000", os.path.join(self.firmwarePath,"main.ino.partitions.bin"), 
                                    "0x10000", os.path.join(self.firmwarePath,"main.ino.bin")]
                            print('Using command %s' % ' '.join(cmd))
                            env = os.environ
                            process = subprocess.Popen(cmd,shell=True, env=env)
                            process.wait()
                        except Exception as e:
                            print(e)
                            print("Firmware not flashed.")
                            return False                    
                print("Firmware flashed")
                return True
            except Exception as e:
                print(e)
                print("Firmware flash failed")
                return False
            
        else:
            try:
                try:
                    cmd = ["esptool.py", 
                            "--chip", "esp32",
                            "--port", self.port, 
                            "--baud", "921600", 
                            "write_flash",
                            "--flash_freq", "80m",
                            "--flash_mode", "dio", 
                            "--flash_size", "detect", 
                            "0xe000", os.path.join(self.firmwarePath,"boot_app0.bin"),
                            "0x1000", os.path.join(self.firmwarePath,"main.ino.bootloader.bin"), 
                            "0x8000", os.path.join(self.firmwarePath,"main.ino.partitions.bin"), 
                            "0x10000", os.path.join(self.firmwarePath,"main.ino.bin")]
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
                            "0xe000", os.path.join(self.firmwarePath,"boot_app0.bin"),
                            "0x1000", os.path.join(self.firmwarePath,"main.ino.bootloader.bin"), 
                            "0x8000", os.path.join(self.firmwarePath,"main.ino.partitions.bin"), 
                            "0x10000", os.path.join(self.firmwarePath,"main.ino.bin")]
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

        print("We are checking for pre-built binaries on Github")
        self.firmwareDownloadPath = 'https://api.github.com/repos/youseetoo/uc2-esp32/releases/latest'
        releaseResponse = requests.get(
            self.firmwareDownloadPath
        )
        latestVersion = releaseResponse.json()['tag_name']
        print("Latest version is: "+latestVersion)

        ## attempting to downloda the current ImSwitch version
        self.downloadURL = releaseResponse.json()['assets'][0]['browser_download_url']
        self.uc2restZip = "UC2Rest.zip"
        print("We are downloading the software from: "+self.uc2restZip)
                

        print("Downloading Firmware from "+self.firmwareDownloadPath)
        # download the firmware from github
        ## inplace replacement won't work I guess? => seems to work
        def dlImSwitch(downloadURL, fileName):
            resultDL = urllib.request.urlretrieve(downloadURL, fileName, MyProgressBar())
            print("Done Downloading")


        try:
            if not os.path.exists(self.firmwarePath):
                os.makedirs(self.firmwarePath)
            # remove file if exists
            if os.path.exists(self.firmwarePath+self.uc2restZip):
                os.remove(os.path.join(self.firmwarePath,self.uc2restZip))
            
            # download the new version in a separate thread
            mThread =  threading.Thread(target=dlImSwitch, args=(self.downloadURL, os.path.join(self.firmwarePath,self.uc2restZip)))
            mThread.start()
            mThread.join()

            print("Succesfully downloaded file: "+os.path.join(self.firmwarePath, self.uc2restZip))
            return True
        except Exception as e:
            print(e)
            print("Firmware download failed"+self.uc2restZip)

            return False
            
    def removeFirmware(self):
        try:
            print("Removing Firmware:"+self.uc2restZip)
            for ifile in glob.glob(self.firmwarePath+"/*"):
                try:os.remove(ifile)
                except:pass
            return True
        except Exception as e:
            print(e)
            return False
        
        

class MyProgressBar():
    def __init__(self):
        self.pbar = None

    def __call__(self, block_num, block_size, total_size):
        if not self.pbar:
            self.pbar=progressbar.ProgressBar(maxval=total_size)
            self.pbar.start()

        downloaded = block_num * block_size
        if downloaded < total_size:
            self.pbar.update(downloaded)
        else:
            self.pbar.finish()
    
        
if __name__ == "__main__":
    updater = updater(port="/dev/cu.SLAB_USBtoUART", firmwarePath=None)
    updater.downloadFirmware()
    updater.flashFirmware()
    
    # remove firmware.bin after flashing
    updater.removeFirmware()
    
            
        
                      