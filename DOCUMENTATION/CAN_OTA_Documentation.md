# CAN OTA Update Module

The CAN OTA (Over-The-Air) update module allows you to remotely update firmware on UC2 CAN satellite devices through WiFi connections.

## Overview

The OTA process consists of these steps:

1. **Send OTA Command**: Tell a CAN device to connect to WiFi and start an OTA server
2. **Device Setup**: The device connects to WiFi and starts ArduinoOTA
3. **Status Response**: Device reports back its IP address and status
4. **Firmware Upload**: Use PlatformIO to upload new firmware wirelessly

## Quick Start

```python
import uc2rest

# Connect to UC2 main controller
ESP32 = uc2rest.UC2Client(serialport="/dev/ttyUSB0")

# Define callback to handle OTA responses
def ota_callback(response):
    if response["success"]:
        print(f"Device {response['canId']} ready at {response['ip']}")
        print(f"Upload with: {ESP32.canota.get_platformio_upload_command(response['canId'])}")
    else:
        print(f"OTA failed: {response['statusMsg']}")

# Register callback
ESP32.canota.register_callback(0, ota_callback)

# Start OTA on Motor X
ESP32.canota.start_motor_ota("X", "WiFiName", "WiFiPassword")
```

## API Reference

### Main Methods

#### `start_ota_update(can_id, ssid, password, timeout=300000)`
Send OTA command to any CAN device.

**Parameters:**
- `can_id` (int): CAN ID of target device
- `ssid` (str): WiFi network name
- `password` (str): WiFi password  
- `timeout` (int): OTA timeout in milliseconds (default: 5 minutes)

**Example:**
```python
ESP32.canota.start_ota_update(20, "MyWiFi", "mypassword")
```

#### Convenience Methods

- `start_motor_ota(axis, ssid, password)` - Motors X/Y/Z (CAN IDs 11/12/13)
- `start_laser_ota(laser_id, ssid, password)` - Laser controllers (CAN ID 20+)
- `start_led_ota(ssid, password)` - LED controller (CAN ID 30)

### Callback System

#### `register_callback(key, callback_function)`

Register functions to handle OTA status updates.

**Callback Function Format:**
```python
def ota_callback(ota_response):
    # ota_response contains:
    # - canId: CAN ID of device
    # - status: 0=success, 1=wifi_failed, 2=ota_failed  
    # - statusMsg: Human readable message
    # - ip: Device IP address (if successful)
    # - hostname: Device hostname (e.g., "UC2-CAN-14.local")
    # - success: True if status == 0
    pass
```

**Callback Keys:**
- `0`: General OTA events (recommended for most use cases)
- `1-9`: Device-specific callbacks (automatically mapped from CAN ID)

### Utility Methods

#### `get_ota_hostname(can_id)`
Get expected hostname for a device in OTA mode.

```python
hostname = ESP32.canota.get_ota_hostname(20)  # Returns "UC2-CAN-14.local"
```

#### `get_platformio_upload_command(can_id)`
Generate PlatformIO upload command for OTA firmware update.

```python
cmd = ESP32.canota.get_platformio_upload_command(20)
# Returns: "platformio run -t upload --upload-port UC2-CAN-14.local"
```

## Device CAN ID Reference

| Device Type | Axis/ID | CAN ID | Convenience Method |
|-------------|---------|--------|-------------------|
| Motor X     | X       | 11     | `start_motor_ota("X", ...)` |
| Motor Y     | Y       | 12     | `start_motor_ota("Y", ...)` |
| Motor Z     | Z       | 13     | `start_motor_ota("Z", ...)` |
| Laser       | 0       | 20     | `start_laser_ota(0, ...)` |
| LED Controller | -    | 30     | `start_led_ota(...)` |

## Status Codes

- **0**: Success - Device connected to WiFi and OTA server started
- **1**: WiFi connection failed - Check SSID/password
- **2**: OTA start failed - Device error

## Complete Example

```python
import uc2rest
import time

# Connect to UC2
ESP32 = uc2rest.UC2Client(serialport="/dev/ttyUSB0", DEBUG=True)

# Setup callback
def handle_ota(response):
    device_id = response["canId"]
    
    if response["success"]:
        print(f"✅ Device {device_id} ready!")
        print(f"   IP: {response['ip']}")
        print(f"   Hostname: {response['hostname']}")
        
        # Generate upload command
        upload_cmd = ESP32.canota.get_platformio_upload_command(device_id)
        print(f"   Upload: {upload_cmd}")
        
    else:
        print(f"❌ Device {device_id} failed: {response['statusMsg']}")

ESP32.canota.register_callback(0, handle_ota)

# Start OTA on multiple devices
devices = [
    ("Motor X", lambda: ESP32.canota.start_motor_ota("X", "WiFi", "pass")),
    ("Motor Y", lambda: ESP32.canota.start_motor_ota("Y", "WiFi", "pass")),
    ("Laser", lambda: ESP32.canota.start_laser_ota(0, "WiFi", "pass")),
    ("LED", lambda: ESP32.canota.start_led_ota("WiFi", "pass"))
]

for name, start_func in devices:
    print(f"Starting OTA for {name}...")
    start_func()
    time.sleep(2)  # Small delay between commands

# Wait for responses
print("Waiting for OTA responses...")
time.sleep(30)
```

## Firmware Upload

After a device reports successful OTA setup:

1. **Verify connection**: Ping the device hostname
   ```bash
   ping UC2-CAN-14.local
   ```

2. **Upload firmware**: Use the generated PlatformIO command
   ```bash
   platformio run -t upload --upload-port UC2-CAN-14.local
   ```

3. **Monitor upload**: Watch for upload progress and completion

## Error Handling

Common issues and solutions:

- **WiFi connection failed**: Check SSID/password, ensure network is reachable
- **OTA start failed**: Device may be busy, try again after a few seconds  
- **Timeout**: Increase timeout parameter or check device connectivity
- **No response**: Verify CAN device is connected and responding to CAN commands

## Integration with Existing Code

The CAN OTA module integrates seamlessly with existing UC2 workflows:

```python
# Normal motor operation
ESP32.motor.move_x(steps=1000, speed=1000)

# Update motor firmware via OTA
ESP32.canota.start_motor_ota("X", "WiFi", "password")

# Continue with normal operation after update
ESP32.motor.move_x(steps=-1000, speed=1000)
```