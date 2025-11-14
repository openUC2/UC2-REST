# Laser Callback Mechanism - Updated

The laser callback mechanism has been updated to follow the same pattern as the motor and home callbacks, providing consistent behavior across all UC2-REST modules.

## Key Changes

### ✅ **Fixed Callback Registration**
- Changed pattern from `"home"` to `"laser"` for proper laser status messages
- Now correctly registers for laser-specific responses

### ✅ **Proper Data Parsing**
- Fixed callback to parse `"laser"` data instead of incorrectly parsing `"home"` data
- Handles both single laser and multiple laser responses
- Updates `laserValues` array instead of non-existent `isHomed` array

### ✅ **Consistent Pattern**
- Follows the same callback pattern as motor and home modules
- Uses the same `_callbackPerKey` structure
- Supports multiple callback registrations (0-9)

## Expected Laser Response Format

The callback now correctly handles laser responses in this format:

### Single Laser Response:
```json
{
    "laser": {
        "LASERid": 1,
        "LASERval": 512,
        "isDone": 1
    },
    "qid": 2
}
```

### Multiple Laser Response:
```json
{
    "laser": [
        {"LASERid": 0, "LASERval": 100, "isDone": 1},
        {"LASERid": 1, "LASERval": 512, "isDone": 1},
        {"LASERid": 2, "LASERval": 300, "isDone": 1}
    ],
    "qid": 3
}
```

## Usage

### Register a Callback
```python
def laser_callback(laser_values):
    """
    Callback function for laser status updates.
    
    :param laser_values: NumPy array [laser0, laser1, laser2, laser3]
    """
    laser_names = ["Laser 0", "Laser 1 (R)", "Laser 2 (G)", "Laser 3 (B)"]
    for i, value in enumerate(laser_values):
        if value > 0:
            print(f"{laser_names[i]}: {value} (ON)")

# Register the callback
ESP32.laser.register_callback(0, laser_callback)
```

### Set Laser and Monitor Response
```python
# Set laser - callback will be triggered when response arrives
ESP32.laser.set_laser(channel=1, value=512, is_blocking=False)

# Blocking mode also triggers callback
ESP32.laser.set_laser(channel=2, value=1023, is_blocking=True)
```

## Laser Values Array

The `laserValues` array maintains the current state of all lasers:
- `laserValues[0]`: Laser 0 (white/general)
- `laserValues[1]`: Laser 1 (red)
- `laserValues[2]`: Laser 2 (green)  
- `laserValues[3]`: Laser 3 (blue)

## Callback Parameters

### Callback Function Signature:
```python
def my_callback(laser_values: np.ndarray) -> None:
    """
    :param laser_values: NumPy array of shape (4,) containing current laser values
    """
    pass
```

### Callback Keys:
- **Key 0**: General laser status updates (recommended for most use cases)
- **Keys 1-9**: Additional callback slots for specific purposes

## Benefits of the Update

### ✅ **Consistency**
- Same pattern as motor and home callbacks
- Predictable behavior across all modules

### ✅ **Reliability**
- Correct pattern registration ensures callbacks are triggered
- Proper data parsing prevents errors

### ✅ **Flexibility**
- Supports both single and multiple laser responses
- Multiple callback registration options

### ✅ **Real-time Monitoring**
- Track laser state changes in real-time
- Immediate feedback on laser operations

## Migration from Old Version

If you were using the laser module before this update:

### Old (broken) behavior:
```python
# This didn't work correctly - registered for "home" pattern
# and tried to parse motor data as laser data
```

### New (fixed) behavior:
```python
# Now works correctly - registers for "laser" pattern
# and properly parses laser response data
def laser_callback(laser_values):
    print(f"Current laser values: {laser_values}")

ESP32.laser.register_callback(0, laser_callback)
ESP32.laser.set_laser(channel=1, value=512)
```

The laser callback mechanism is now fully functional and consistent with the rest of the UC2-REST framework.