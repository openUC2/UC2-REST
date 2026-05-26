import time
import json
import re
import zlib
import threading
from pathlib import Path

try:
    import serial
    HAS_SERIAL = True
except ImportError:
    HAS_SERIAL = False


gTIMEOUT = 10  # seconds to wait for a response from the ESP32

# ============================================================================
# CANopen OTA (serial -> ESP32 master -> CAN) protocol constants
# ----------------------------------------------------------------------------
# Mirrors tools/ota/canopen_ota_serial.py in uc2-ESP. The master accepts a
# JSON preamble ``/ota_start`` and then expects the raw firmware bytes
# streamed over serial. After each CHUNK_SIZE bytes it answers with
# ``{"ota_rx": <total_bytes>}`` until the final ``{"ota_status": "success"}``.
# ============================================================================

STREAMING_BAUD = 921600       # match canopen_ota_serial.py default
CHUNK_SIZE = 4096             # bytes per ACK-paced write
READY_TIMEOUT_S = 10.0        # wait for {"ota_status":"ready"}
FLASH_TIMEOUT_S = 600.0       # SDO domain transfer can take minutes
ACK_TIMEOUT_S = 30.0          # per-chunk wait for {"ota_rx": N}
INTER_CHUNK_DELAY_S = 0.0     # >0 only if host overruns master's UART

# Match standalone JSON objects on a single line.
_JSON_LINE_RE = re.compile(rb"\{[^\n]*\}")


class CANOTA(object):
    """
    CAN OTA (Over-The-Air) update controller for UC2 satellite devices.
    
    This class handles sending OTA commands to CAN slave devices and processing
    the responses through callbacks. It manages the WiFi connection setup and
    OTA server initialization on remote devices.
    
    The OTA process involves:
    1. Sending OTA command with WiFi credentials to a specific CAN device
    2. Device connects to WiFi and starts ArduinoOTA server
    3. Device sends back acknowledgment and IP address via serial
    4. Device becomes available as UC2-CAN-<HEXID>.local for firmware upload
    """
    
    def __init__(self, parent=None, nCallbacks=10):
        """
        Initialize the CAN OTA controller.
        
        :param parent: Parent UC2Client object with post_json and serial interfaces
        :param nCallbacks: Number of callback slots to initialize (default: 10)
        """
        self._parent = parent
        self.nCallbacks = nCallbacks
        
        # Initialize callback functions for different types of OTA events
        self.init_callback_functions(self.nCallbacks)
        
        # Register callback function for OTA status messages on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_ota_status, pattern="ota")
    
    def init_callback_functions(self, nCallbacks=10):
        """
        Initialize the callback function dictionary.
        
        :param nCallbacks: Number of callback slots to create
        """
        self._callbackPerKey = {}
        self.nCallbacks = nCallbacks
        for i in range(nCallbacks):
            self._callbackPerKey[i] = None
    
    def _callback_ota_status(self, data):
        """
        Process incoming OTA status messages from CAN devices.
        
        Expected message format:
        {
            "ota": {
                "canId": 20,
                "status": 0,
                "statusMsg": "Success",
                "ip": "192.168.2.137",
                "hostname": "UC2-CAN-14.local"
            }
        }
        
        Status codes:
        - 0: Success (WiFi connected, OTA server started)
        - 1: WiFi connection failed
        - 2: OTA start failed
        
        :param data: JSON data containing OTA status information
        """
        try:
            ota_data = data["ota"]
            can_id = ota_data.get("canId")
            status = ota_data.get("status")
            status_msg = ota_data.get("statusMsg", "")
            ip_address = ota_data.get("ip", "")
            hostname = ota_data.get("hostname", "")
            
            # Create a structured response for callbacks
            ota_response = {
                "canId": can_id,
                "status": status,
                "statusMsg": status_msg,
                "ip": ip_address,
                "hostname": hostname,
                "success": status == 0
            }
            
            # Trigger callbacks for different events
            # Key 0: General OTA status updates
            if callable(self._callbackPerKey[0]):
                self._callbackPerKey[0](ota_response)
            
            # Key for specific CAN ID (use can_id % nCallbacks to avoid overflow)
            if can_id is not None:
                callback_key = (can_id % (self.nCallbacks - 1)) + 1  # Reserve key 0 for general
                if callable(self._callbackPerKey[callback_key]):
                    self._callbackPerKey[callback_key](ota_response)
                    
        except Exception as e:
            print(f"Error in _callback_ota_status: {e}")
    
    def register_callback(self, key, callback_function):
        """
        Register a callback function for OTA events.
        
        :param key: Callback key (0 for general events, 1-9 for specific CAN IDs)
        :param callback_function: Function to call when OTA event occurs
                                Function should accept one parameter: ota_response dict
        
        Example:
            def my_ota_callback(ota_response):
                if ota_response["success"]:
                    print(f"Device {ota_response['canId']} ready at {ota_response['ip']}")
                else:
                    print(f"OTA failed for device {ota_response['canId']}: {ota_response['statusMsg']}")
            
            ESP32.canota.register_callback(0, my_ota_callback)
        """
        if 0 <= key < self.nCallbacks:
            self._callbackPerKey[key] = callback_function
        else:
            raise ValueError(f"Callback key must be between 0 and {self.nCallbacks-1}")
    
    def start_ota_update(self, can_id, ssid, password, timeout=300000, is_blocking=False, 
                        response_timeout=gTIMEOUT):
        """
        Send OTA command to a specific CAN slave device.
        
        This tells the slave to connect to WiFi and start an OTA server.
        The device will become available as UC2-CAN-<HEXID>.local for firmware upload.
        
        :param can_id: CAN ID of the target device (e.g., 11 for Motor X, 20 for Laser)
        :param ssid: WiFi network name
        :param password: WiFi password
        :param timeout: OTA timeout in milliseconds (default: 300000 = 5 minutes)
        :param is_blocking: If True, wait for acknowledgment response
        :param response_timeout: Timeout for response in seconds
        :return: Response from the device (if blocking)
        
        Examples:
            # Motor X (CAN ID 11)
            ESP32.canota.start_ota_update(11, "WiFi", "pass123")
            
            # Laser (CAN ID 20) with custom timeout
            ESP32.canota.start_ota_update(20, "WiFi", "pass123", timeout=600000)
        """
        path = "/can_act"
        payload = {
            "task": path,
            "ota": {
                "canid": can_id,
                "ssid": ssid,
                "password": password,
                "timeout": timeout
            }
        }
        
        # Send the payload to the parent, which handles the actual communication
        nResponses = 1 if is_blocking else 0
        return self._parent.post_json(
            path,
            payload,
            getReturn=is_blocking,
            timeout=response_timeout if is_blocking else 0,
            nResponses=nResponses
        )
    
    def start_motor_ota(self, motor_axis, ssid, password, timeout=300000, is_blocking=False):
        """
        Convenience method for starting OTA on motor controllers.
        
        :param motor_axis: Motor axis ("X", "Y", "Z") or CAN ID (11, 12, 13)
        :param ssid: WiFi network name
        :param password: WiFi password
        :param timeout: OTA timeout in milliseconds
        :param is_blocking: If True, wait for acknowledgment response
        :return: Response from the device
        
        Examples:
            ESP32.canota.start_motor_ota("X", "WiFi", "pass123")
            ESP32.canota.start_motor_ota(11, "WiFi", "pass123")  # Same as above
        """
        # Map motor axes to CAN IDs
        motor_can_ids = {
            "X": 11,
            "Y": 12, 
            "Z": 13
        }
        
        if isinstance(motor_axis, str):
            motor_axis = motor_axis.upper()
            if motor_axis not in motor_can_ids:
                raise ValueError(f"Invalid motor axis '{motor_axis}'. Use 'X', 'Y', or 'Z'")
            can_id = motor_can_ids[motor_axis]
        else:
            can_id = motor_axis
        
        return self.start_ota_update(can_id, ssid, password, timeout, is_blocking)
    
    def start_led_ota(self, ssid, password, timeout=300000, is_blocking=False):
        """
        Convenience method for starting OTA on LED controller (CAN ID 30).
        
        :param ssid: WiFi network name
        :param password: WiFi password
        :param timeout: OTA timeout in milliseconds
        :param is_blocking: If True, wait for acknowledgment response
        :return: Response from the device
        """
        return self.start_ota_update(30, ssid, password, timeout, is_blocking)
    
    def start_laser_ota(self, laser_id=0, ssid="", password="", timeout=300000, is_blocking=False):
        """
        Convenience method for starting OTA on laser controller.
        
        :param laser_id: Laser ID (0 for main laser, maps to CAN ID 20)
        :param ssid: WiFi network name
        :param password: WiFi password
        :param timeout: OTA timeout in milliseconds
        :param is_blocking: If True, wait for acknowledgment response
        :return: Response from the device
        """
        # Map laser ID to CAN ID (for now, laser 0 = CAN ID 20)
        can_id = 20 + laser_id
        return self.start_ota_update(can_id, ssid, password, timeout, is_blocking)
    
    def get_ota_hostname(self, can_id):
        """
        Generate the expected hostname for a CAN device in OTA mode.
        
        :param can_id: CAN ID of the device
        :return: Expected hostname (e.g., "UC2-CAN-14.local" for CAN ID 20)
        """
        hex_id = format(can_id, 'X')  # Convert to hexadecimal
        return f"UC2-CAN-{hex_id}.local"
    
    def get_platformio_upload_command(self, can_id, project_path="."):
        """
        Generate the PlatformIO upload command for OTA firmware update.
        
        :param can_id: CAN ID of the target device
        :param project_path: Path to the PlatformIO project (default: current directory)
        :return: PlatformIO upload command string
        
        Example:
            cmd = ESP32.canota.get_platformio_upload_command(20)
            # Returns: "platformio run -t upload --upload-port UC2-CAN-14.local"
        """
        hostname = self.get_ota_hostname(can_id)
        if project_path != ".":
            return f"platformio run -t upload --upload-port {hostname} -d {project_path}"
        else:
            return f"platformio run -t upload --upload-port {hostname}"

    # ========================================================================
    # CANopen OTA streaming (serial -> ESP32 master -> CAN)
    # ----------------------------------------------------------------------
    # Mirrors tools/ota/canopen_ota_serial.py: send the ``/ota_start`` JSON
    # preamble, wait for ``{"ota_status":"ready"}``, then stream raw firmware
    # bytes in CHUNK_SIZE blocks, blocking after each block until the master
    # acknowledges with ``{"ota_rx": <total_bytes>}``. Verify CRC32 (not
    # MD5) and wait for ``{"ota_status":"success"|"error"}``.
    # ========================================================================

    def start_can_streaming_ota(self, can_id: int, firmware_path: str,
                                 progress_callback=None, status_callback=None,
                                 port: str = None, baud: int = STREAMING_BAUD):
        """
        Upload firmware to a CAN slave via the CANopen OTA path
        (laptop -> serial -> ESP32 master -> CAN -> slave).

        :param can_id: Target slave CANopen node ID (e.g., 11 for Motor X)
        :param firmware_path: Path to the firmware binary (.bin file)
        :param progress_callback: Function(chunk_idx, total_chunks, bytes_sent, speed_kbps)
        :param status_callback: Function(status_str, success_bool)
        :param port: Serial port (default: use parent's port)
        :param baud: Baud rate for streaming (default: 115200)
        :return: Thread handle; join + read .result for success/failure.
        """
        if not HAS_SERIAL:
            if status_callback:
                status_callback("Serial library not available", False)
            return False

        firmware_path = Path(firmware_path)
        if not firmware_path.exists():
            if status_callback:
                status_callback(f"Firmware not found: {firmware_path}", False)
            return False

        if port is None and self._parent and hasattr(self._parent, "serial"):
            port = self._parent.serial.serialport

        if not port:
            if status_callback:
                status_callback("No serial port specified", False)
            return False

        firmware_data = firmware_path.read_bytes()
        if not firmware_data:
            if status_callback:
                status_callback("Firmware file is empty", False)
            return False

        firmware_size = len(firmware_data)
        crc32 = zlib.crc32(firmware_data) & 0xFFFFFFFF
        num_chunks = (firmware_size + CHUNK_SIZE - 1) // CHUNK_SIZE

        if status_callback:
            status_callback(
                f"Firmware: {firmware_size} bytes, {num_chunks} chunks, "
                f"CRC32: 0x{crc32:08X}", True)

        upload_thread = threading.Thread(
            target=self._streaming_upload_worker,
            args=(can_id, port, baud, firmware_data, firmware_size,
                  crc32, num_chunks, progress_callback, status_callback)
        )
        upload_thread.daemon = True
        upload_thread.start()

        return upload_thread

    def start_can_streaming_ota_blocking(self, can_id: int, firmware_path: str,
                                          progress_callback=None, status_callback=None,
                                          port: str = None, baud: int = STREAMING_BAUD):
        """Blocking variant of :meth:`start_can_streaming_ota`."""
        thread = self.start_can_streaming_ota(
            can_id, firmware_path, progress_callback, status_callback, port, baud
        )
        if isinstance(thread, threading.Thread):
            thread.join()
            return getattr(thread, 'result', False)
        return False

    def _streaming_upload_worker(self, can_id, port, baud, firmware_data, firmware_size,
                                  crc32, num_chunks,
                                  progress_callback, status_callback):
        """Worker thread for the ``/ota_start`` streaming upload."""
        ser = None
        parent_serial_was_open = False
        result = False

        try:
            # Step 1: Close parent's serial connection temporarily so we can
            # take exclusive control of the port.
            if self._parent and hasattr(self._parent, "serial"):
                parent_serial = self._parent.serial
                if parent_serial.serialdevice and parent_serial.serialdevice.isOpen():
                    parent_serial_was_open = True
                    if status_callback:
                        status_callback("Closing parent serial connection...", True)
                    parent_serial.closeSerial()
                    time.sleep(1.0)

            # Step 2: Open raw serial connection.
            if status_callback:
                status_callback(
                    f"Opening streaming connection on {port} at {baud} baud...", True)
            if hasattr(serial, "tools") and hasattr(serial.tools, "list_ports_common") \
                    and isinstance(port, serial.tools.list_ports_common.ListPortInfo):
                port = port.device
            ser = serial.Serial(port, baud, timeout=0.05)
            time.sleep(0.5)
            ser.reset_input_buffer()

            # Step 3: Send the JSON preamble and wait for ``ready``.
            cmd = json.dumps({
                "task": "/ota_start",
                "ota": {
                    "nodeId": can_id,
                    "size": firmware_size,
                    "crc32": f"0x{crc32:08X}",
                },
            })
            if status_callback:
                status_callback(f"Sending /ota_start for node {can_id}...", True)
            print(f"  TX: {cmd}")
            ser.write((cmd + "\n").encode())
            ser.flush()

            ready = self._wait_for_status(
                ser, time.time() + READY_TIMEOUT_S, key_values=("ready",),
                status_callback=status_callback)
            if not ready or ready.get("ota_status") != "ready":
                if status_callback:
                    status_callback(f"Master did not become ready: {ready}", False)
                return
            if status_callback:
                status_callback(f"Master ready: {ready}", True)

            # Step 4: Stream raw firmware bytes — strictly ACK-paced.
            if status_callback:
                status_callback(
                    f"Uploading {firmware_size} bytes "
                    f"(ACK-paced, {CHUNK_SIZE} B chunks)...", True)

            start_time = time.time()
            sent = 0
            last_ack = 0
            ack_buf = bytearray()
            chunk_no = 0

            while sent < firmware_size:
                end = min(sent + CHUNK_SIZE, firmware_size)
                payload = firmware_data[sent:end]
                chunk_no += 1
                t_send = time.time()
                ser.write(payload)
                ser.flush()
                sent = end

                # Block until the master ACKs at least ``sent`` bytes.
                deadline = time.time() + ACK_TIMEOUT_S
                while last_ack < sent:
                    last_ack, err = self._drain_acks(ser, ack_buf, last_ack)
                    if err is not None:
                        if status_callback:
                            status_callback(
                                f"Master reported error at chunk #{chunk_no}: {err}",
                                False)
                        time.sleep(0.3)
                        self._drain_acks(ser, ack_buf, last_ack)
                        return
                    if last_ack >= sent:
                        break
                    if time.time() > deadline:
                        if status_callback:
                            status_callback(
                                f"Timeout waiting for ACK at chunk #{chunk_no} "
                                f"(sent={sent}, last_ack={last_ack})", False)
                        time.sleep(0.3)
                        self._drain_acks(ser, ack_buf, last_ack)
                        return
                    time.sleep(0.002)

                if progress_callback:
                    elapsed = time.time() - start_time
                    speed = sent / elapsed / 1024 if elapsed > 0 else 0
                    progress_callback(chunk_no, num_chunks, sent, speed)

                if INTER_CHUNK_DELAY_S > 0:
                    time.sleep(INTER_CHUNK_DELAY_S)

            elapsed = time.time() - start_time
            if elapsed > 0 and status_callback:
                status_callback(
                    f"Transfer: {elapsed:.1f}s "
                    f"({firmware_size / elapsed / 1024:.1f} KB/s)", True)

            # Step 5: Wait for the flashing -> success/error transition.
            if status_callback:
                status_callback(
                    "Waiting for slave flash result (may take minutes)...", True)
            final = self._wait_for_status(
                ser, time.time() + FLASH_TIMEOUT_S,
                key_values=("success", "error"),
                status_callback=status_callback)
            if final is None:
                if status_callback:
                    status_callback("Timed out waiting for OTA result", False)
                return
            if final.get("ota_status") == "success":
                if status_callback:
                    status_callback(
                        "OTA flash successful — slave will reboot.", True)
                result = True
            else:
                if status_callback:
                    status_callback(f"OTA flash failed: {final}", False)

        except Exception as e:
            if status_callback:
                status_callback(f"Error: {str(e)}", False)

        finally:
            if ser and ser.isOpen():
                ser.close()

            if parent_serial_was_open and self._parent and hasattr(self._parent, "serial"):
                if status_callback:
                    status_callback("Restoring serial connection...", True)
                time.sleep(1.0)
                try:
                    self._parent.serial.openDevice(port, baud)
                except Exception as e:
                    if status_callback:
                        status_callback(
                            f"Warning: Could not restore serial: {e}", False)

        threading.current_thread().result = result

    # ========================================================================
    # Streaming protocol helpers (mirror canopen_ota_serial.py)
    # ========================================================================

    @staticmethod
    def _try_extract_json_status(buf: bytes):
        """Return the first JSON object on ``buf`` containing ``ota_status``
        or ``ota_rx``, else None."""
        for match in _JSON_LINE_RE.findall(buf):
            try:
                obj = json.loads(match.decode(errors="ignore"))
            except json.JSONDecodeError:
                continue
            if isinstance(obj, dict) and ("ota_status" in obj or "ota_rx" in obj):
                return obj
        return None

    def _wait_for_status(self, ser, deadline, key_values=None,
                          status_callback=None):
        """Read until ``key_values`` matches ``ota_status`` or deadline elapses."""
        buf = bytearray()
        while time.time() < deadline:
            chunk = ser.read(ser.in_waiting or 1)
            if chunk:
                buf.extend(chunk)
                obj = self._try_extract_json_status(bytes(buf))
                if obj is not None:
                    if key_values is None:
                        return obj
                    status = obj.get("ota_status")
                    if status in key_values:
                        return obj
                    if status == "error":
                        return obj
                    idx = bytes(buf).rfind(b"}")
                    if idx > 0:
                        del buf[: idx + 1]
            else:
                time.sleep(0.005)
        return None

    def _drain_acks(self, ser, ack_buf: bytearray, last_ack: int):
        """Read pending bytes, return (latest_ack, error_obj_or_None)."""
        latest = last_ack
        err = None
        if ser.in_waiting:
            ack_buf.extend(ser.read(ser.in_waiting))
        for match in _JSON_LINE_RE.findall(bytes(ack_buf)):
            try:
                obj = json.loads(match.decode(errors="ignore"))
            except json.JSONDecodeError:
                continue
            if "ota_rx" in obj:
                try:
                    latest = max(latest, int(obj["ota_rx"]))
                except (TypeError, ValueError):
                    pass
            elif obj.get("ota_status") == "error":
                err = obj
        # Drop processed lines so we don't re-match them forever.
        nl = ack_buf.rfind(b"\n")
        if nl >= 0:
            del ack_buf[: nl + 1]
        return latest, err