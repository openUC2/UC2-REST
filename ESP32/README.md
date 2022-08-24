## Work with Arduino-cli


```
arduino-cli core update-index --config-file arduino-cli.yaml
arduino-cli core install esp32:esp32
arduino-cli board list

# compile
#arduino-cli compile --fqbn esp32:esp32:esp32-poe-iso .

arduino-cli compile --fqbn esp32:esp32:esp32:PSRAM=disabled,PartitionScheme=huge_app,CPUFreq=80 main/main.ino  --output-dir ./build/ --libraries ./libraries/


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
