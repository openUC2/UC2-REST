import time
import json
import struct
import hashlib
import threading
from pathlib import Path

try:
    import serial
    HAS_SERIAL = True
except ImportError:
    HAS_SERIAL = False


gTIMEOUT = 10  # seconds to wait for a response from the ESP32

# ============================================================================
# CAN OTA STREAMING PROTOCOL CONSTANTS
# ============================================================================

# Protocol constants (must match CanOtaStreaming.h)
SYNC_1 = 0xAA
SYNC_2 = 0x55

# Stream message types
STREAM_START  = 0x70
STREAM_DATA   = 0x71
STREAM_ACK    = 0x72
STREAM_NAK    = 0x73
STREAM_FINISH = 0x74
STREAM_ABORT  = 0x75
STREAM_STATUS = 0x76


# Protocol parameters
PORT = "/dev/cu.SLAB_USBtoUART"
BAUD = 921600
CAN_ID = 11

# Streaming protocol constants
PAGE_SIZE = 4096       # 4KB pages (flash-aligned)
CHUNK_SIZE = 512       # 512 bytes per chunk within page
CHUNKS_PER_PAGE = PAGE_SIZE // CHUNK_SIZE  # 8 chunks per page

PAGE_ACK_TIMEOUT = 10.0
MAX_PAGE_RETRIES = 3
MAX_SESSION_RETRIES = 2

# Default baud rate for streaming
STREAMING_BAUD = 921600


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
    # CAN OTA STREAMING (USB-based, no WiFi required)
    # ========================================================================
    
    def start_can_streaming_ota(self, can_id: int, firmware_path: str, 
                                 progress_callback=None, status_callback=None,
                                 port: str = None, baud: int = STREAMING_BAUD):
        """
        Upload firmware to a CAN slave device via USB->CAN streaming protocol.
        
        This method:
        1. Sends streaming start command via JSON to initialize the slave
        2. Temporarily closes the parent's serial connection
        3. Opens a raw serial connection for binary streaming
        4. Uploads the firmware in 4KB pages with ACK per page
        5. Verifies MD5 checksum
        6. Restores the parent's serial connection
        
        :param can_id: CAN ID of the target device (e.g., 11 for Motor X)
        :param firmware_path: Path to the firmware binary (.bin file)
        :param progress_callback: Function(page, total_pages, bytes_sent, speed_kbps)
        :param status_callback: Function(status_str, success_bool)
        :param port: Serial port (default: use parent's port)
        :param baud: Baud rate for streaming (default: 921600)
        :return: True if successful, False otherwise
        
        Example:
            def on_progress(page, total, bytes_sent, speed):
                print(f"Page {page}/{total} - {speed:.1f} KB/s")
            
            def on_status(msg, success):
                print(f"Status: {msg} ({'OK' if success else 'FAIL'})")
            
            ESP32.canota.start_can_streaming_ota(
                11, "/path/to/firmware.bin",
                progress_callback=on_progress,
                status_callback=on_status
            )
        """
        if not HAS_SERIAL:
            if status_callback:
                status_callback("Serial library not available", False)
            return False
        
        # Resolve firmware path
        firmware_path = Path(firmware_path)
        if not firmware_path.exists():
            if status_callback:
                status_callback(f"Firmware not found: {firmware_path}", False)
            return False
        
        # Get port from parent if not specified
        if port is None and self._parent and hasattr(self._parent, "serial"):
            port = self._parent.serial.serialport
        
        if not port:
            if status_callback:
                status_callback("No serial port specified", False)
            return False
        
        # Load firmware
        firmware_data = firmware_path.read_bytes()
        firmware_size = len(firmware_data)
        md5_hex = hashlib.md5(firmware_data).hexdigest()
        md5_bytes = bytes.fromhex(md5_hex)
        num_pages = (firmware_size + PAGE_SIZE - 1) // PAGE_SIZE
        
        if status_callback:
            status_callback(f"Firmware: {firmware_size} bytes, {num_pages} pages, MD5: {md5_hex[:16]}...", True)
        
        # Run streaming upload in a separate thread to not block
        upload_thread = threading.Thread(
            target=self._streaming_upload_worker,
            args=(can_id, port, baud, firmware_data, firmware_size, 
                  md5_hex, md5_bytes, num_pages, progress_callback, status_callback)
        )
        upload_thread.daemon = True
        upload_thread.start()
        
        return upload_thread
    
    def start_can_streaming_ota_blocking(self, can_id: int, firmware_path: str,
                                          progress_callback=None, status_callback=None,
                                          port: str = None, baud: int = STREAMING_BAUD):
        """
        Blocking version of start_can_streaming_ota.
        Waits for upload to complete before returning.
        
        :return: True if successful, False otherwise
        """
        thread = self.start_can_streaming_ota(
            can_id, firmware_path, progress_callback, status_callback, port, baud
        )
        if isinstance(thread, threading.Thread):
            thread.join()
            return getattr(thread, 'result', False)
        return False
    
    def _streaming_upload_worker(self, can_id, port, baud, firmware_data, firmware_size,
                                  md5_hex, md5_bytes, num_pages, 
                                  progress_callback, status_callback):
        """Worker thread for streaming upload."""
        ser = None
        parent_serial_was_open = False
        result = False
        
        try:
            # Step 1: Close parent's serial connection temporarily
            if self._parent and hasattr(self._parent, "serial"):
                parent_serial = self._parent.serial
                if parent_serial.serialdevice and parent_serial.serialdevice.isOpen():
                    parent_serial_was_open = True
                    if status_callback:
                        status_callback("Closing parent serial connection...", True)
                    parent_serial.closeSerial()
                    for i in range(10): time.sleep(0.1) # wait a bit
            
            # Step 2: Open raw serial connection
            if status_callback:
                status_callback(f"Opening streaming connection on {port} at {baud} baud...", True)
            
            ser = serial.Serial(port, baud, timeout=0.1)#, write_timeout=1.0)
            time.sleep(0.5)
            self._drain_serial(ser)
            
            # Step 3: Send streaming start command via JSON
            if status_callback:
                status_callback(f"Initializing streaming session for CAN ID {can_id}...", True)
            
            start_cmd = {
                "task": "/can_ota_stream",
                "canid": can_id,
                "action": "start",
                "firmware_size": firmware_size,
                "page_size": PAGE_SIZE,
                "chunk_size": CHUNK_SIZE,
                "md5": md5_hex
            }
            print("  Sending STREAM_START command...", start_cmd)
            response = self._send_json(ser, start_cmd)
            # {"task": "/can_ota_stream", "canid": 11, "action": "start", "firmware_size": 876784, "page_size": 4096, "chunk_size": 512, "md5": "43ba96b4d18c010201762b840476bf83", "qid": 1}
            if response and  str(response).find("success")<=0:
                print("  ERROR: STREAM_START command failed!")
                print(f"  Response: {response}")
                return False
            print("  ✓ Streaming session started")
            '''b\'[326584][I][CanOtaStreaming.cpp:427] actFromJsonStreaming(): Target CAN ID: 11\\r\\n[326586][I][CanOtaStreaming.cpp:456] actFromJsonStreaming(): Stream OTA START to CAN ID 11: size=876784, page_size=4096, chunk_size=512\\r\\n[326588][I][CanOtaStreaming.cpp:464] actFromJsonStreaming(): Relaying STREAM_START to slave 0x0B\\r\\n[326590][I][CanOtaStreaming.cpp:653] startStreamToSlave(): Starting stream to slave 0x0B: 876784 bytes, 215 pages\\r\\n[326968][I][CanOtaStreaming.cpp:622] handleSlaveStreamResponse(): Slave STREAM_ACK: page=65535, bytes=0, nextSeq=0\\r\\n++\\n{"success":true,"qid":1}\\n--\\n\\x00\''''
            #self._drain_serial(ser)
            #self._send_json(ser, {"task": "/can_act", "debug": 1})

            if status_callback:
                status_callback("Streaming session started, uploading...", True)
            
            # Step 4: Stream pages
            start_time = time.time()
            seq = 0
            bytes_sent = 0
            failed_pages = 0

            for page_idx in range(num_pages):
                page_start = page_idx * PAGE_SIZE
                page_end = min(page_start + PAGE_SIZE, firmware_size)
                page_data = firmware_data[page_start:page_end]
                
                # Pad last page if necessary
                if len(page_data) < PAGE_SIZE:
                    page_data = page_data + bytes(PAGE_SIZE - len(page_data))
                
                # Send page with retry
                success, seq = self._send_page_with_retry(ser=ser, page_idx=page_idx, page_data=page_data, seq_start=seq)
                bytes_sent += PAGE_SIZE
                
                if not success:
                    if status_callback:
                        status_callback(f"Page {page_idx} failed after retries", False)
                    return False
                
                # Progress callback
                if progress_callback:
                    elapsed = time.time() - start_time
                    speed = bytes_sent / elapsed / 1024 if elapsed > 0 else 0
                    progress_callback(page_idx + 1, num_pages, bytes_sent, speed)
            
            # Step 5: Send FINISH command
            if status_callback:
                status_callback("All pages sent, verifying MD5...", True)
            
            finish_packet = self._build_stream_finish_packet(md5_bytes)
            ser.write(finish_packet)
            ser.flush()
            
            # Wait for final verification
            success, raw = self._wait_for_final_ack(ser, num_pages - 1, timeout=30.0)
            
            if success:
                elapsed = time.time() - start_time
                speed = firmware_size / elapsed / 1024
                if status_callback:
                    status_callback(f"SUCCESS! {firmware_size} bytes in {elapsed:.1f}s ({speed:.1f} KB/s)", True)
                result = True
            else:
                # Check for reboot indicator
                try:
                    raw_text = raw.decode('utf-8', errors='replace') if raw else ""
                    if "Rebooting" in raw_text or "OTA COMPLETE" in raw_text:
                        if status_callback:
                            status_callback("Device rebooting - OTA successful!", True)
                        result = True
                    else:
                        if status_callback:
                            status_callback("MD5 verification failed", False)
                except:
                    if status_callback:
                        status_callback("Final verification failed", False)
        
        except Exception as e:
            if status_callback:
                status_callback(f"Error: {str(e)}", False)
            
            # Try to abort
            if ser and ser.isOpen():
                try:
                    self._send_json(ser, {"task": "/can_ota_stream", "canid": can_id, "action": "abort"})
                except:
                    pass
        
        finally:
            # Close streaming connection
            if ser and ser.isOpen():
                ser.close()
            
            # Restore parent's serial connection
            if parent_serial_was_open and self._parent and hasattr(self._parent, "serial"):
                if status_callback:
                    status_callback("Restoring serial connection...", True)
                time.sleep(1.0)
                try:
                    self._parent.serial.openDevice(port, baud)
                except Exception as e:
                    if status_callback:
                        status_callback(f"Warning: Could not restore serial: {e}", False)
        
        # Store result in thread for blocking version
        threading.current_thread().result = result
    
    # ========================================================================
    # Streaming Protocol Helper Methods
    # ========================================================================
    
    def _drain_serial(self, ser, label="", verbose=True):
        """Read and display any pending data"""
        time.sleep(0.05)
        data = b''
        while ser.in_waiting:
            data += ser.read(ser.in_waiting)
            time.sleep(0.01)
        if data and verbose:
            print(f"  [{label}] Drained {len(data)} bytes")
        return data
    
    def _send_json(self, ser, data, wait_response=True, timeout=2.0):
        """Send JSON command over serial."""
        json_str = json.dumps(data, separators=(',', ':'))
        tx_data = (json_str + '\n').encode()
        print(f"  TX: {json_str[:80]}{'...' if len(json_str) > 80 else ''}")
        ser.write(tx_data)
        ser.flush()
        
        if wait_response:
            time.sleep(0.3)
            start = time.time()
            response = b''
            while time.time() - start < timeout:
                if ser.in_waiting:
                    ser_line = ser.read(ser.in_waiting)
                    print(ser_line)
                    response += ser_line
                    if b'{"success":' in response or b'{"error":' in response:
                        break
                time.sleep(0.05)
            print(f"  RX: {len(response)} bytes")
            return response
        return None
    
    def _build_stream_data_packet(self, page_idx, offset, seq, chunk_data):
        """Build a binary stream data packet."""
        header = struct.pack('<HHHH', page_idx, offset, len(chunk_data), seq)
        packet_body = bytes([STREAM_DATA]) + header + chunk_data
        checksum = sum(packet_body) & 0xFF
        return bytes([SYNC_1, SYNC_2]) + packet_body + bytes([checksum])
    
    def _build_stream_finish_packet(self, md5_bytes):
        """Build STREAM_FINISH packet with MD5 hash."""
        header = bytes([STREAM_FINISH])
        packet_body = header + md5_bytes
        checksum = sum(packet_body) & 0xFF
        return bytes([SYNC_1, SYNC_2]) + packet_body + bytes([checksum])
    
    def _wait_for_session_start(self, ser, timeout=10.0):
        """Wait for streaming session start ACK."""
        start = time.time()
        buffer = bytearray()
        
        while time.time() - start < timeout:
            if ser.in_waiting:
                buffer.extend(ser.read(ser.in_waiting))
                
                # Look for STREAM_ACK
                for i in range(len(buffer) - 5):
                    if buffer[i] == SYNC_1 and buffer[i+1] == SYNC_2:
                        if buffer[i+2] == STREAM_ACK and buffer[i+3] == 0:  # status OK
                            return True
            else:
                time.sleep(0.01)
        
        return False
    
    def _wait_for_page_ack(self, ser, expected_page, timeout=PAGE_ACK_TIMEOUT):
        """
        Wait for STREAM_ACK response.
        Returns: (success, last_complete_page, bytes_received, raw_response)
        """
        start = time.time()
        buffer = bytearray()
        
        while time.time() - start < timeout:
            if ser.in_waiting:
                new_data = ser.read(ser.in_waiting)
                buffer.extend(new_data)
                
                # Check for log messages indicating success (for final ACK)
                try:
                    text = bytes(buffer).decode('utf-8', errors='replace')
                    if "OTA COMPLETE" in text or "Rebooting" in text:
                        return (True, expected_page, 0, bytes(buffer))
                except:
                    pass
                
                # Search for STREAM_ACK response
                # Format: [SYNC][CMD][status][canId][lastPage_L][lastPage_H][bytes(4)][nextSeq(2)][reserved(2)]
                i = 0
                while i < len(buffer) - 15:
                    if buffer[i] == SYNC_1 and buffer[i+1] == SYNC_2:
                        cmd = buffer[i+2]
                        
                        if cmd == STREAM_ACK:
                            status = buffer[i+3]
                            can_id = buffer[i+4]
                            last_page = buffer[i+5] | (buffer[i+6] << 8)  # Little endian
                            bytes_recv = struct.unpack('<I', bytes(buffer[i+7:i+11]))[0]
                            next_seq = buffer[i+11] | (buffer[i+12] << 8)
                            
                            if status == 0:  # CAN_OTA_OK
                                return (True, last_page, bytes_recv, bytes(buffer))
                            else:
                                print(f"  STREAM_ACK with error status: {status}")
                                return (False, last_page, bytes_recv, bytes(buffer))
                        
                        elif cmd == STREAM_NAK:
                            status = buffer[i+3]
                            can_id = buffer[i+4]
                            error_page = buffer[i+5] | (buffer[i+6] << 8)
                            missing_offset = buffer[i+7] | (buffer[i+8] << 8)
                            
                            print(f"  STREAM_NAK: status={status}, page={error_page}, offset={missing_offset}")
                            return (False, error_page, missing_offset, bytes(buffer))
                    
                    i += 1
            else:
                time.sleep(0.001)
        
        return (False, -1, 0, bytes(buffer))
    
    def _wait_for_final_ack(self, ser, expected_page, timeout=30.0):
        """Wait for final ACK after FINISH command."""
        start = time.time()
        buffer = bytearray()
        
        while time.time() - start < timeout:
            if ser.in_waiting:
                buffer.extend(ser.read(ser.in_waiting))
                
                # Look for success indicators
                for i in range(len(buffer) - 5):
                    if buffer[i] == SYNC_1 and buffer[i+1] == SYNC_2:
                        if buffer[i+2] == STREAM_ACK and buffer[i+3] == 0:
                            return (True, bytes(buffer))
            else:
                time.sleep(0.01)
        
        return (False, bytes(buffer))
    
    def _send_page_with_retry(self, ser, page_idx, page_data, seq_start, 
                               max_retries=MAX_PAGE_RETRIES):
        """Send a page with retry logic."""
        for retry in range(max_retries):
            seq = seq_start
            
            # Send all chunks for this page
            for chunk_idx in range(CHUNKS_PER_PAGE):
                chunk_start = chunk_idx * CHUNK_SIZE
                chunk_end = chunk_start + CHUNK_SIZE
                chunk_data = page_data[chunk_start:chunk_end]
                offset = chunk_idx * CHUNK_SIZE
                
                packet = self._build_stream_data_packet(page_idx, offset, seq, chunk_data)
                ser.write(packet)
                seq += 1
            
            ser.flush()
            
            # Wait for ACK
            success, acked_page, acked_bytes, raw = self._wait_for_page_ack(ser, page_idx)
            
            if success:
                return (True, seq)
            
            if retry < max_retries - 1:
                time.sleep(0.5)
                self._drain_serial(ser)
        
        return (False, seq)