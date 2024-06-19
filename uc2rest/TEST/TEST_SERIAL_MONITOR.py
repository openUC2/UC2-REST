#!/usr/bin/env python3

import sys
import threading
import queue
import serial

class PythonSerialMonitor:
    def __init__(self):
        self.windows = False
        self.unix = False
        self.fd = None
        self.old_settings = None

        try:
            # Windows
            import msvcrt
            self.windows = True
        except ImportError:
            # Unix
            import tty, termios
            self.fd = sys.stdin.fileno()
            self.old_settings = termios.tcgetattr(self.fd)
            tty.setcbreak(self.fd)
            self.unix = True

        self.input_queue = queue.Queue()
        self.stop_queue = queue.Queue()
        self.pause_queue = queue.Queue()

        self.input_thread = threading.Thread(target=self.add_input, args=(self.input_queue,self.stop_queue,self.pause_queue,))
        self.input_thread.daemon = True
        self.input_thread.start()

    def getch(self):
        if self.unix:
            import tty, termios
            try:
                tty.setcbreak(sys.stdin.fileno())
                ch = sys.stdin.read(1)
            finally:
                termios.tcsetattr(self.fd, termios.TCSADRAIN, self.old_settings)
            return ch
        if self.windows:
            import msvcrt
            return msvcrt.getch().decode()

    def cleanUp(self):
        if self.unix:
            import termios
            termios.tcsetattr(self.fd, termios.TCSADRAIN, self.old_settings)

    def add_input(self, input_queue, stop_queue, pause_queue):
        while True:
            input_queue.put(self.getch())
            if not pause_queue.empty():
                if pause_queue.get() == 'pause':
                    while True:
                        if not pause_queue.empty():
                            if pause_queue.get() == 'resume':
                                break
            if not stop_queue.empty():
                if stop_queue.get() == 'stop':
                    break

    def run(self):
        port = "/dev/cu.wchusbserial110"  # Adjust this to your device's serial port
        baud = 500000
        baud = 9600
        baseports = ['/dev/ttyUSB', '/dev/ttyACM', 'COM', '/dev/tty.usbmodem12341']
        self.ser = None

        while not self.ser:
            for baseport in baseports:
                if self.ser:
                    break
                for i in range(0, 64):
                    try:
                        #port = baseport + str(i)
                        self.ser = serial.Serial(port, baud, timeout=1)
                        print("Monitor: Opened " + port + '\r')
                        break
                    except:
                        self.ser = None
                        pass

            if not self.ser:
                print("Monitor: Couldn't open a serial port.")
                print("Monitor: Press 'enter' to try again or 'esc' to exit.")
                while True:
                    if not self.input_queue.empty():
                        keyboardInput = self.input_queue.get()
                        if ord(keyboardInput) == 27:
                            self.stop_queue.put('stop')
                            self.cleanUp()
                            sys.exit(1)
                        else:
                            break

        self.ser.flushInput()

        while True:
            if not self.input_queue.empty():
                keyboardInput = self.input_queue.get()
                import json
                keyboardInput = json.dumps({"task":"/state_get"})
                print("Keyboard: " + keyboardInput)
                self.ser.write(keyboardInput.encode())  # Encode string to bytes


            try:
                #bytesToRead = self.ser.inWaiting()  # get the amount of bytes available at the input queue
                if 1:
                    line = self.ser.readline().decode('utf-8')  # read the bytes and decode
                    print("Teensy: " + line.strip())
            except IOError:
                # Manually raise the error again so it can be caught outside of this method
                raise IOError()

if __name__ == "__main__":
    psm = PythonSerialMonitor()

    while True:
        try:
            psm.run()
        except serial.SerialException:
            print("Monitor: Disconnected (Serial exception)")
        except IOError:
            print("Monitor: Disconnected (I/O Error)")
        except KeyboardInterrupt:
            print("Monitor: Keyboard Interrupt. Exiting Now...")
            sys.exit(1)
