## Work with Arduino-cli


```
arduino-cli core update-index --config-file arduino-cli.yaml
arduino-cli core install esp32:esp32
arduino-cli board list

# compile
#arduino-cli compile --fqbn esp32:esp32:esp32-poe-iso .

mkdir build
arduino-cli compile --fqbn esp32:esp32:esp32:PSRAM=disabled,PartitionScheme=huge_app,CPUFreq=80 ESP32/main/main.ino  --output-dir ./build/ --libraries ./libraries/


esptool.py --chip esp32 --port /dev/cu.SLAB_USBtoUART --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 /build/main.ino.bootloader.bin 0x8000 /build/main.ino.partitions.bin 0xe000 /build/boot_app0.bin 0x10000 /build/main.ino.bin 

# upload
arduino-cli upload -p /dev/cu.usbserial-1310 --fqbn esp32:esp32:esp32-poe-iso .




# screen the serial monitor
screen /dev/cu.usbserial-1310 115200
```

Upload via esptool:

```
pip install esptool
```

```
python -m esptool --chip esp32 --port /dev/cu.SLAB_USBtoUART --baud 921600  write_flash --flash_mode dio --flash_size detect 0x0 main.ino.bin
```
