# QID-Based Done/Busy Tracking & Pause/Resume

## Overview

The QID (Query ID) tracking system provides reliable command completion tracking for the UC2 firmware and Python API. Instead of counting serial responses, the firmware's **QidRegistry** tracks internal sub-actions and automatically emits a single `{"qid":X,"state":"done"}` message when all sub-actions for a command complete.

### Key Benefits
- **Reliable**: No more counting `nResponses` — firmware knows when actions are truly done
- **Multi-axis aware**: A 3-axis motor command sends one "done" when all 3 axes finish
- **Pause/Resume**: Motor movements can be paused mid-motion and resumed later
- **CAN-transparent**: Works identically for local motors and CAN-bus satellite motors
- **Backward compatible**: Legacy `nResponses` mode remains the default

---

## Architecture

```
Python API                    ESP32 Master                   CAN Slave
─────────                     ────────────                   ─────────
                              
sendMessage(cmd, qid=5)  →   SerialProcess receives JSON
                              MotorJsonParser.act() extracts qid
                              QidRegistry::registerQid(5, 3)  ← 3 axes
                              serialize(qid) → ACK              
                         ←    {"qid":5,"success":1}
                              
                              FocusMotor starts 3 axes
                              Axis 0 (local): stepper runs...
                              Axis 1 (CAN):   ──────────────→  Motor runs
                              Axis 2 (CAN):   ──────────────→  Motor runs
                              
                              Axis 0 stops: reportActionDone(5) → 1/3
                              Axis 1 CAN response:          ←  MotorState{qid=5}
                                reportActionDone(5) → 2/3
                              Axis 2 CAN response:          ←  MotorState{qid=5}
                                reportActionDone(5) → 3/3 = DONE!
                         ←    {"qid":5,"state":"done"}
                              
event.wait() returns     
```

---

## Firmware API

### QID States

| State     | Description |
|-----------|-------------|
| `busy`    | Action is in progress |
| `done`    | All sub-actions completed successfully |
| `paused`  | Motors paused, can be resumed |
| `timeout` | No completion within timeout (default 30s) |
| `error`   | Error occurred (e.g. hard limit triggered) |

### Endpoints

#### Query QID State
```json
{"task": "/qid_state", "qid": 5}
```
Response:
```json
{"qid": 5, "state": "done"}
```

#### Pause a Running QID
```json
{"task": "/qid_pause", "qid": 5}
```
- Stops all motors associated with QID 5
- Saves remaining steps for each axis
- For CAN motors: sends stop, waits 500ms for position update
- Response: `{"qid": 5, "state": "paused"}`

#### Resume a Paused QID
```json
{"task": "/qid_resume", "qid": 5}
```
- Restarts paused motors with remaining distance
- Uses relative positioning for remaining steps
- Response: `{"qid": 5, "state": "done"}` (when resumed motion completes)

### Automatic Notifications

The firmware automatically sends these JSON messages over serial:

```json
{"qid": 5, "state": "done"}     // All actions completed
{"qid": 5, "state": "timeout"}  // Timeout after 30s (configurable)
{"qid": 5, "state": "error"}    // Hard limit or other error
{"qid": 5, "state": "paused"}   // Paused via /qid_pause
```

### Supported Controllers

| Controller | Type | QID Behavior |
|-----------|------|-------------|
| Motor     | Async | Register with N axes, done when all stop |
| Laser     | Sync  | Register + done immediately in act() |
| LED       | Sync  | Register + done immediately in act() |
| Galvo     | Async | (Future: register in scanner task) |

---

## Python API

### Enabling QID-Done Mode

```python
from uc2rest import UC2Client

# Legacy mode (default) - uses nResponses counting
uc2 = UC2Client(serialport="/dev/ttyUSB0")

# QID-done mode - waits for firmware "done" event
uc2.serial.use_qid_done = True
```

### Motor Movement

```python
# With use_qid_done=True:
# - Sends motor command with QID
# - Blocks until firmware sends {"qid":X,"state":"done"}
# - No nResponses counting needed
uc2.motor.move_stepper(steps=(1000, 2000, 0), speed=(20000, 20000, 0))

# With use_qid_done=False (default, legacy):
# - nResponses = len(steppers) + 1
# - Blocks until that many serial responses arrive
uc2.motor.move_stepper(steps=(1000, 2000, 0), speed=(20000, 20000, 0))
```

### Pause/Resume from Python

```python
# Start a long movement
uc2.serial.use_qid_done = True
# (in a thread or non-blocking)
uc2.motor.move_stepper(steps=(100000, 0, 0), speed=(5000, 0, 0))

# Pause mid-motion
uc2.serial.post_json("/qid_pause", {"qid": last_qid})

# Resume later
uc2.serial.post_json("/qid_resume", {"qid": last_qid})
```

### Query State

```python
response = uc2.serial.post_json("/qid_state", {"qid": 5})
# Returns: [{"qid": 5, "state": "done"}]
```

---

## Implementation Details

### QidRegistry (Firmware)

- Located in `main/src/qid/QidRegistry.h` and `.cpp`
- Circular buffer of 32 entries (configurable via `QID_REGISTRY_SIZE`)
- Thread-safe: protected by FreeRTOS mutex
- Eviction policy: when full, evicts oldest DONE/TIMEOUT/ERROR entry
- Default timeout: 30 seconds (`QID_DEFAULT_TIMEOUT_MS`)
- Timeout check runs in `FocusMotor::loop()` via `tickTimeout()`

### CAN Protocol Change

The `MotorState` struct (packed, sent over CAN) now includes a `qid` field:

```cpp
#pragma pack(push,1)
struct MotorState {
    int32_t currentPosition = 0;
    bool isRunning = 0;
    uint8_t axis = 0;
    int16_t qid = -1;  // NEW: QID for tracking
};
#pragma pack(pop)
```

**IMPORTANT**: This changes the CAN message size. Both master and slave firmware must be updated together.

### Files Modified

| File | Changes |
|------|---------|
| `main/src/qid/QidRegistry.h` | New - Registry header |
| `main/src/qid/QidRegistry.cpp` | New - Registry implementation |
| `main/src/motor/MotorTypes.h` | Added `qid` to `MotorState` |
| `main/src/motor/FocusMotor.cpp` | reportActionDone in stopStepper, tickTimeout in loop, reportActionError in startStepper |
| `main/src/motor/MotorJsonParser.cpp` | registerQid in parseMotorDriveJson |
| `main/src/laser/LaserController.cpp` | register+reportDone in act() |
| `main/src/led/LedController.cpp` | register+reportDone in act() |
| `main/src/can/can_controller.cpp` | QID propagation in MotorState, reportActionDone on slave response |
| `main/src/serial/SerialProcess.cpp` | New endpoint handlers, QidRegistry::setup() |
| `main/src/wifi/Endpoints.h` | New endpoint constants |
| `uc2rest/mserial.py` | use_qid_done flag, event-based blocking |
| `uc2rest/motor.py` | Conditional nResponses |

---

## Testing

### Basic Motor Test
```json
{"task": "/motor_act", "motor": {"steppers": [{"stepperid": 1, "position": 10000, "speed": 20000, "isabs": 0}]}, "qid": 5}
```
Expected:
1. Immediate: `{"qid":5,"success":1}`
2. Motor position updates during motion
3. On completion: `{"qid":5,"state":"done"}`

### Multi-Axis Test
```json
{"task": "/motor_act", "motor": {"steppers": [{"stepperid": 0, "position": 5000, "speed": 10000}, {"stepperid": 1, "position": 10000, "speed": 20000}, {"stepperid": 2, "position": 3000, "speed": 15000}]}, "qid": 7}
```
Expected: Single `{"qid":7,"state":"done"}` after ALL 3 axes finish.

### Laser Test (Synchronous)
```json
{"task": "/laser_act", "LASERid": 0, "LASERval": 512, "qid": 10}
```
Expected: Immediate `{"qid":10,"state":"done"}` (laser is synchronous).

### Pause/Resume Test
```json
{"task": "/motor_act", "motor": {"steppers": [{"stepperid": 1, "position": 100000, "speed": 5000}]}, "qid": 15}
// Wait 2 seconds...
{"task": "/qid_pause", "qid": 15}
// Response: {"qid":15,"state":"paused"}
// Wait...
{"task": "/qid_resume", "qid": 15}
// Motor completes remaining distance
// Response: {"qid":15,"state":"done"}
```

### Timeout Test
Send motor command, physically block motor for >30 seconds:
```json
{"qid":X,"state":"timeout"}
```

### State Query
```json
{"task": "/qid_state", "qid": 15}
// Response: {"qid":15,"state":"busy"} or "done", "paused", etc.
```
