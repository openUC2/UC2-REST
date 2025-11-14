import time
import json


gTIMEOUT = 10  # seconds to wait for a response from the ESP32


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