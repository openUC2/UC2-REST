import serial
import threading
import time
import json

# Configuration parameters
port = "/dev/cu.wchusbserial110"  # Adjust this to your device's serial port
baudrate = 500000

# Open the serial port
ser = serial.Serial(port, baudrate, timeout=1)

lock = threading.Lock()
# Function to handle incoming messages
def read_from_port(ser):
    while True:
        try:
            with lock:
                reading = ser.readline().decode('utf-8').rstrip()
                if reading:
                    print(f"Received: {reading}")
        except Exception as e:
            print("Error reading from port")
            break
# Start the thread for reading
thread = threading.Thread(target=read_from_port, args=(ser,))
thread.start()

iiter = 0
try:
    print("Serial Terminal Running. Type 'exit' to quit.")
    while True:
        iiter+=1
        # Get input from the user
        input_data = {'task': '/ledarr_act', 'led': {'LEDArrMode': 0, 'led_array': [{'id': 0, 'r': 25, 'g': 42, 'b': 4}, {'id': 1, 'r': 52, 'g': 26, 'b': 11}, {'id': 2, 'r': 51, 'g': 10, 'b': 17}]}, 'qid': 1}
        if input_data == 'exit':
            break
        # Send data if the user typed something
        if input_data:
            with lock:
                ser.write(f"{json.dumps(input_data)}\n".encode('utf-8'))
                print(f"Sent: {iiter}")
finally:
    ser.close()
    print("Serial Terminal Closed.")
