#%%
import uc2rest
import time

port = "unknown"
port = "/dev/cu.SLAB_USBtoUART"
ESP32 = uc2rest.UC2Client(serialport=port, baudrate=500000, DEBUG=True)

# register callback function for key/value pair 
def my_callback_key1(value):
    print("Callback: ", value)
ESP32.message.register_callback(1, my_callback_key1)

while True:
    time.sleep(.1)