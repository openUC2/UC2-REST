import serial
import json
import threading
import time

class SerialMonitor:
    def __init__(self, port, baud_rate=115200):
        self.ser = serial.Serial(port, baud_rate)
        self.command_id = 0
        self.pending_responses = {}
        self.lock = threading.Lock()
        self.running = True
        self.thread = threading.Thread(target=self._process_commands)
        self.thread.start()

    def _process_commands(self):
        while self.running:
            response = self.ser.readline().decode('utf-8').strip()
            json_response = json.loads(response)
            response_id = json_response.get("id", None)

            if response_id is not None:
                with self.lock:
                    event = self.pending_responses.get(response_id, None)
                    if event:
                        event["response"] = json_response
                        event["event"].set()

            time.sleep(0.1)

    def send_command(self, command):
        event = threading.Event()
        with self.lock:
            self.command_id += 1
            command["id"] = self.command_id
            self.pending_responses[self.command_id] = {"event": event, "response": None}

        json_command = json.dumps(command)
        self.ser.write(json_command.encode('utf-8'))
        self.ser.write(b'\n') # To mark the end of command
        return self.command_id

    def wait_for_response(self, command_id, timeout=None):
        with self.lock:
            event_data = self.pending_responses.get(command_id, None)

        if event_data:
            event = event_data["event"]
            event.wait(timeout)
            with self.lock:
                response = self.pending_responses.pop(command_id, None)
                if response:
                    return response["response"]

        return None

    def stop(self):
        self.running = False
        self.thread.join()
        self.ser.close()

# Usage example
monitor = SerialMonitor('/dev/cu.SLAB_USBtoUART') # Change to your port

command_to_send = {
    "command": "ping"
}

command_id = monitor.send_command(command_to_send)
response = monitor.wait_for_response(command_id)

if response:
    print("Response:", response)

monitor.stop()
