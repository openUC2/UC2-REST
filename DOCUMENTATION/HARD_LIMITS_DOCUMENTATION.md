# Hard Limits Documentation

## Overview

Hard limits provide emergency stop functionality when a motor hits an endstop during normal operation (not during homing). When triggered, the motor immediately stops and the position is set to `999999` to indicate an error state. The user must perform homing to clear the error state and re-establish position tracking.

## Features

- **Emergency Stop**: Automatically stops motor when endstop is triggered
- **CAN Bus Support**: Full support for controlling hard limits via CAN bus
- **Polarity Configuration**: Supports both Normally Open (NO) and Normally Closed (NC) endstops
- **Per-Axis Configuration**: Each axis (A, X, Y, Z) can be configured independently
- **Status Monitoring**: Check if hard limit has been triggered
- **Homing Integration**: Hard limits are ignored during homing, triggered flag is cleared after homing

## Architecture

### Firmware (ESP32)

**Data Structures** ([MotorTypes.h](../uc2-ESP/main/src/motor/MotorTypes.h)):
```cpp
struct MotorSettings {
    bool hardLimitEnabled = true;   // Enabled by default
    bool hardLimitPolarity = 0;     // 0 = NO, 1 = NC
    // ... other settings
};

struct MotorData {
    bool hardLimitEnabled = true;
    bool hardLimitPolarity = 0;
    bool hardLimitTriggered = false;  // Set to true when endstop hit
    // ... other data
};
```

**Key Functions** ([FocusMotor.cpp](../uc2-ESP/main/src/motor/FocusMotor.cpp)):
```cpp
void setHardLimit(int axis, bool enabled, bool polarity);
void clearHardLimitTriggered(int axis);
void checkHardLimits();  // Called continuously in loop
```

**CAN Bus Support** ([can_controller.cpp](../uc2-ESP/main/src/can/can_controller.cpp)):
- Hard limit settings are transmitted via `MotorSettings` struct
- Settings are sent to CAN slaves automatically when modified
- `sendMotorSettingsToCANDriver()` transmits all motor settings including hard limits

### REST API

**Endpoint**: `/motor_act`

**Set Hard Limits**:
```json
{
    "task": "/motor_act",
    "hardlimits": {
        "steppers": [{
            "stepperid": 1,
            "enabled": 1,
            "polarity": 0
        }]
    }
}
```

**Clear Hard Limit Triggered Flag**:
```json
{
    "task": "/motor_act",
    "hardlimits": {
        "steppers": [{
            "stepperid": 1,
            "clear": 1
        }]
    }
}
```

**Get Hard Limit Status** (via `/motor_get`):
```json
{
    "steppers": [{
        "stepperid": 1,
        "hardLimitEnabled": true,
        "hardLimitPolarity": 0,
        "hardLimitTriggered": false
    }]
}
```

### Python API

**Module**: `uc2rest.motor.Motor`

#### Set Hard Limits

```python
motor.set_hard_limits(axis="X", enabled=True, polarity=0, timeout=1)
```

**Parameters**:
- `axis`: Motor axis (0/"A", 1/"X", 2/"Y", 3/"Z")
- `enabled`: `True` to enable protection, `False` to disable
- `polarity`: 
  - `0` = Normally Open (NO) - endstop LOW when not pressed, HIGH when pressed
  - `1` = Normally Closed (NC) - endstop HIGH when not pressed, LOW when pressed
- `timeout`: Command timeout in seconds

**Example**:
```python
# Enable hard limit with normally open endstop
motor.set_hard_limits(axis="X", enabled=True, polarity=0)

# Disable hard limit protection
motor.set_hard_limits(axis="Z", enabled=False)

# Configure for normally closed endstop
motor.set_hard_limits(axis="Y", enabled=True, polarity=1)
```

#### Clear Hard Limit Flag

```python
motor.clear_hard_limit(axis="X", timeout=1)
```

Clears the triggered flag. Typically done automatically during homing.

#### Get Hard Limit Status

```python
# Get status for specific axis
x_limits = motor.get_hard_limits(axis="X")
# Returns: {"axis": 1, "enabled": True, "polarity": 0, "triggered": False}

# Get status for all axes
all_limits = motor.get_hard_limits()
```

#### Check if Triggered

```python
# Check single axis
if motor.is_hard_limit_triggered(axis="X"):
    print("X-axis hard limit triggered! Performing homing...")
    motor.home_x()

# Check all axes
triggered = motor.is_hard_limit_triggered()
for ax, is_triggered in triggered.items():
    if is_triggered:
        print(f"Axis {ax} needs homing!")
```

## CAN Bus Implementation

### Master Side (Host Controller)

When hard limits are configured on the master:
1. Settings are stored in `MotorData` structure
2. `FocusMotor::setHardLimit()` saves to preferences
3. `sendMotorSettingsToCANDriver()` is called automatically
4. `MotorSettings` struct (including hard limit config) is transmitted via CAN

**Code Flow**:
```cpp
// In FocusMotor::setHardLimit()
getData()[axis]->hardLimitEnabled = enabled;
getData()[axis]->hardLimitPolarity = polarity;

#if defined(CAN_BUS_ENABLED) && !defined(CAN_RECEIVE_MOTOR)
    MotorSettings settings = can_controller::extractMotorSettings(*getData()[axis]);
    can_controller::sendMotorSettingsToCANDriver(settings, axis);
#endif
```

### Slave Side (CAN Motor Controller)

When slave receives `MotorSettings`:
1. Size check identifies message type: `sizeof(MotorSettings)`
2. Hard limit settings are extracted and applied
3. Settings are stored in local `MotorData` structure
4. `checkHardLimits()` monitors endstop state continuously

**Code Flow** ([can_controller.cpp](../uc2-ESP/main/src/can/can_controller.cpp)):
```cpp
if (size == sizeof(MotorSettings)) {
    MotorSettings receivedMotorSettings;
    memcpy(&receivedMotorSettings, pData, size);
    
    FocusMotor::getData()[mStepper]->hardLimitEnabled = receivedMotorSettings.hardLimitEnabled;
    FocusMotor::getData()[mStepper]->hardLimitPolarity = receivedMotorSettings.hardLimitPolarity;
}
```

### Hard Limit Checking

Performed continuously in motor loop on both master and slave:
```cpp
void checkHardLimits() {
#if defined(CAN_RECEIVE_MOTOR) && defined(MOTOR_CONTROLLER) && defined(DIGITAL_IN_CONTROLLER)
    // Slave: Check local endstops
    // If triggered: stop motor, set position to 999999
#elif defined(MOTOR_CONTROLLER) && defined(DIGITAL_IN_CONTROLLER) && !defined(CAN_BUS_ENABLED)
    // Standalone: Check local endstops
#endif
}
```

## Endstop Polarity

The polarity setting determines how the endstop signal is interpreted:

### Normally Open (NO) - `polarity = 0`
- **Not pressed**: Signal is LOW (0V)
- **Pressed**: Signal is HIGH (3.3V)
- Most common configuration

### Normally Closed (NC) - `polarity = 1`
- **Not pressed**: Signal is HIGH (3.3V)
- **Pressed**: Signal is LOW (0V)
- Safer: wire break = triggered state

## Behavior

### Normal Operation
1. Motor moves normally within soft limits
2. Hard limits are checked continuously
3. If endstop triggered and hard limits enabled:
   - Motor stops immediately
   - Position set to `999999` (error indicator)
   - `hardLimitTriggered` flag set to `true`

### During Homing
1. Hard limits are **ignored** during homing
2. `isHoming` flag bypasses hard limit checks
3. After successful homing:
   - `hardLimitTriggered` flag is cleared
   - Position is reset to home position

### Recovery from Hard Limit
```python
# Check if hard limit was triggered
if motor.is_hard_limit_triggered(axis="X"):
    # Option 1: Clear flag manually (requires manual positioning)
    motor.clear_hard_limit(axis="X")
    motor.set_position(axis="X", position=0)
    
    # Option 2: Perform homing (recommended)
    motor.home_x()  # Automatically clears flag
```

## Implementation Details

### Supported Configurations

| Configuration | Master | Slave | Hard Limit Support |
|--------------|--------|-------|-------------------|
| Standalone | ✅ | N/A | ✅ Local endstops |
| CAN Master | ✅ | ✅ | ✅ Via CAN |
| CAN Slave | N/A | ✅ | ✅ Local endstops |
| CAN Hybrid | ✅ | ✅ | ✅ Both local & CAN |

### Configuration Detection

The system automatically determines whether to use local or CAN-based hard limits:

**Hybrid Mode** (some axes local, some via CAN):
```cpp
bool shouldUseCANForAxis(int axis) {
#if defined(CAN_BUS_ENABLED) && defined(CAN_SEND_COMMANDS) && defined(CAN_HYBRID)
    // Check if axis has native driver (pin >= 0)
    // If no native driver, use CAN
#endif
}
```

### Persistence

Hard limit settings are saved to non-volatile storage (NVS preferences):
```cpp
preferences.begin("UC2", false);
preferences.putBool(("hlEn" + String(axis)).c_str(), enabled);
preferences.putBool(("hlPol" + String(axis)).c_str(), polarity);
preferences.end();
```

Settings persist across reboots.

## Testing

### Test Sequence

```python
from uc2rest import UC2Client

# Initialize
client = UC2Client()
motor = client.motor

# Test 1: Configure hard limits
print("Configuring hard limits...")
motor.set_hard_limits(axis="X", enabled=True, polarity=0)

# Test 2: Check configuration
status = motor.get_hard_limits(axis="X")
print(f"Hard limit status: {status}")
assert status["enabled"] == True
assert status["polarity"] == 0

# Test 3: Move and trigger (manual)
print("Move motor toward endstop...")
motor.move_x(steps=-10000, speed=5000, is_blocking=False)
# Wait for trigger...

# Test 4: Check triggered state
if motor.is_hard_limit_triggered(axis="X"):
    print("Hard limit triggered! Position:", motor.get_position(axis="X"))
    # Should show position = 999999

# Test 5: Recovery via homing
print("Performing homing to recover...")
motor.home_x()
print("Position after homing:", motor.get_position(axis="X"))

# Test 6: Disable hard limits
motor.set_hard_limits(axis="X", enabled=False)
print("Hard limits disabled")
```

### CAN Bus Testing

```python
# Test CAN slave hard limit control
motor.set_hard_limits(axis="A", enabled=True, polarity=0)  # Axis A via CAN
motor.set_hard_limits(axis="X", enabled=True, polarity=0)  # Axis X local

# Move both axes
motor.move_xyza(steps=(1000, 1000, 0, 1000), speed=(5000, 5000, 0, 5000))

# Check status
all_limits = motor.get_hard_limits()
for limit in all_limits:
    print(f"Axis {limit['axis']}: enabled={limit['enabled']}, triggered={limit['triggered']}")
```

## Troubleshooting

### Hard Limit Not Triggering

**Check**:
1. Verify endstop is wired correctly
2. Check polarity setting matches endstop type (NO vs NC)
3. Verify `DIGITAL_IN_CONTROLLER` is defined in firmware
4. Test endstop signal with `motor.get_hard_limits()` while manually pressing

### False Triggers

**Check**:
1. Verify polarity setting (NO vs NC)
2. Check for electrical noise on endstop line
3. Add pull-up/pull-down resistors if needed
4. Test endstop signal stability

### CAN Slave Not Responding

**Check**:
1. Verify CAN bus wiring and termination
2. Check CAN slave firmware has `CAN_RECEIVE_MOTOR` defined
3. Verify axis mapping (A=0, X=1, Y=2, Z=3)
4. Monitor CAN bus traffic with debug logs

### Position Stuck at 999999

**Solution**:
```python
# Perform homing to clear error state
motor.home_x()

# Or manually clear if you know the position
motor.clear_hard_limit(axis="X")
motor.set_position(axis="X", position=<known_position>)
```

## Related Documentation

- [Homing Documentation](HOME_XY_Usage.md)
- [CAN OTA Documentation](CAN_OTA_Documentation.md)
- [Soft Limits Documentation](SOFT_LIMITS_DOCUMENTATION.md) (if exists)

## Changelog

- **2025-01-30**: Initial documentation created
- **2024-xx-xx**: Hard limit feature implemented with CAN bus support
