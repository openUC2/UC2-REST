import serial
import threading
import time

class ESP32Monitor:
    def __init__(self, port='/dev/cu.SLAB_USBtoUART', baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.serialdevice = serial.Serial(self.port, self.baudrate, timeout=1)
        self.monitoring = True

    def monitor_serial(self):
        while self.monitoring:
            if self.serialdevice.in_waiting:
                line = self.serialdevice.readline().decode('utf-8', errors='ignore').strip()
                print(line)

    def start_monitoring(self):
        # Create and start the monitoring thread
        self.monitor_thread = threading.Thread(target=self.monitor_serial)
        self.monitor_thread.daemon = True  # Makes the thread exit when the main program exits
        self.monitor_thread.start()

    def stop_monitoring(self):
        # Stop the monitoring and join the thread
        self.monitoring = False
        self.monitor_thread.join()

    def restart_esp(self):
        # Send signals to restart the ESP32
        self.serialdevice.setDTR(False)
        self.serialdevice.setRTS(True)
        time.sleep(0.1)
        self.serialdevice.setDTR(False)
        self.serialdevice.setRTS(False)
        time.sleep(0.5)
        print("ESP32 has been restarted.")

    def close(self):
        self.serialdevice.close()

if __name__ == "__main__":
    monitor = ESP32Monitor()

    try:
        monitor.start_monitoring()
        print("Monitoring ESP32 output...")

        # Simulate some operation before restarting ESP32
        time.sleep(1)  # Wait for 5 seconds before sending the reset command
        print("Sending restart command to ESP32...")
        monitor.restart_esp()

        # Keep monitoring for a while after the restart
        time.sleep(10)  # Monitor for 10 seconds after the reset

    except KeyboardInterrupt:
        print("Exiting...")
    finally:
        monitor.stop_monitoring()
        monitor.close()
