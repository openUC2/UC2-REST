import serial
import json
import queue
import threading
import time
import logging
from serial.tools import list_ports

class SerialInterface:
    def __init__(self, baudrate=115200, timeout=5, identity="UC2_Feather"):
        self.baudrate = baudrate
        self.timeout = timeout
        self.identity = identity
        self.serial_device = None
        self.serial_port = None
        self.running = False
        self.command_queue = queue.Queue()
        self.response_queue = queue.Queue()
        self.lock = threading.Lock()

        # Command tracking
        self.identifier_counter = 0
        self.responses = {}

        # Logging setup
        self.logger = logging.getLogger("SerialInterface")
        self.logger.setLevel(logging.DEBUG)
        handler = logging.StreamHandler()
        handler.setFormatter(logging.Formatter('%(asctime)s - %(levelname)s - %(message)s'))
        self.logger.addHandler(handler)

        # Discover device on startup
        self.auto_discover_device()

    def auto_discover_device(self):
        """Auto-discover and initialize the correct serial device."""
        self.logger.info("Starting auto-discovery for serial device...")
        for port in list_ports.comports():
            try:
                self.serial_device = serial.Serial(port.device, self.baudrate, timeout=self.timeout)
                time.sleep(1)  # Allow time for the device to initialize
                if self.handshake():
                    # If handshake succeeds, set the port and start threads
                    self.serial_port = port.device
                    self.logger.info(f"Device found at {self.serial_port}")
                    self.start_threads()
                    return
                else:
                    self.serial_device.close()
            except Exception as e:
                self.logger.warning(f"Failed to open port {port.device}: {e}")
        
        self.logger.error("Device not found. Please check the connection.")
        self.serial_device = None

    def handshake(self):
        """Perform handshake by sending a state_get command to check device identity."""
        self.logger.debug("Attempting handshake...")
        handshake_message = {"task": "/state_get"}
        try:
            self.serial_device.write(json.dumps(handshake_message).encode('utf-8'))
            self.serial_device.write(b'\n')

            # Wait for a response with retries
            for _ in range(5):
                line = self.serial_device.readline().decode('utf-8').strip()
                if line:
                    try:
                        response = json.loads(line)
                        # Confirm the expected device identity
                        if response.get("identifier_name") == self.identity:
                            self.logger.info("Handshake successful.")
                            return True
                    except json.JSONDecodeError:
                        pass
            self.logger.warning("Handshake failed.")
            return False
        except serial.SerialException as e:
            self.logger.error(f"Error during handshake: {e}")
            return False

    def start_threads(self):
        """Initialize threads for command sending and response processing."""
        self.running = True
        self.command_thread = threading.Thread(target=self._send_commands, daemon=True)
        self.response_thread = threading.Thread(target=self._process_responses, daemon=True)
        self.command_thread.start()
        self.response_thread.start()
        self.logger.info("Serial communication threads started.")

    def send_command(self, command_dict):
        """Add a command to the queue and assign a unique identifier."""
        identifier = self._generate_identifier()
        command_dict["qid"] = identifier
        self.command_queue.put(json.dumps(command_dict))
        return identifier

    def _generate_identifier(self):
        """Generate a unique identifier for each command."""
        with self.lock:
            self.identifier_counter += 1
            return self.identifier_counter

    def _send_commands(self):
        """Thread to send commands from the command queue."""
        while self.running:
            try:
                command = self.command_queue.get(timeout=1)
                if command:
                    with self.lock:
                        self.serial_device.write(command.encode('utf-8'))
                        self.logger.debug(f"Sent command: {command}")
                    self.command_queue.task_done()
                time.sleep(0.05)
            except queue.Empty:
                pass
            except serial.SerialException as e:
                self.logger.error(f"Serial write error: {e}")
                self._reconnect()

    def _process_responses(self):
        """Thread to read and process responses from the serial device."""
        buffer = ""
        while self.running:
            try:
                if self.serial_device.in_waiting > 0:
                    line = self.serial_device.readline().decode('utf-8').strip()
                    if line == "++":
                        buffer = ""  # Start of JSON message
                    elif line == "--":
                        # End of JSON message
                        try:
                            response = json.loads(buffer)
                            self.response_queue.put(response)
                            self.logger.debug(f"Received response: {response}")
                        except json.JSONDecodeError:
                            self.logger.error("JSON decoding error in response.")
                        buffer = ""  # Clear buffer after processing
                    else:
                        buffer += line
                time.sleep(0.01)
            except serial.SerialException as e:
                self.logger.error(f"Serial read error: {e}")
                self._reconnect()

    def get_response(self, identifier, timeout=1):
        """Retrieve response matching the identifier from the response queue."""
        end_time = time.time() + timeout
        while time.time() < end_time:
            try:
                response = self.response_queue.get_nowait()
                if response.get("qid") == identifier:
                    return response
            except queue.Empty:
                time.sleep(0.1)
        self.logger.warning(f"Timeout waiting for response with identifier {identifier}")
        return None

    def _reconnect(self):
        """Reconnect to the serial device."""
        self.logger.info("Reconnecting...")
        self.running = False
        if self.serial_device:
            self.serial_device.close()
        time.sleep(1)  # Wait before attempting reconnection
        self.auto_discover_device()  # Attempt auto-discovery again

    def close(self):
        """Close the serial connection and stop threads."""
        self.running = False
        if self.serial_device:
            self.serial_device.close()
        self.logger.info("Serial interface closed.")

# Example Usage
if __name__ == "__main__":
    # Initialize with auto-discovery
    interface = SerialInterface()

    # Example command after successful discovery
    if interface.serial_device:
        identifier = interface.send_command({"task": "/state_get"})
        response = interface.get_response(identifier, timeout=2)
        print("Response:", response)

    # Clean up
    interface.close()
