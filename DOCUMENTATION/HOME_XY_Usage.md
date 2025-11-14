# Home XY - Multi-Motor Homing

The `home_xy` function allows you to home multiple motors simultaneously, which is more efficient than homing them individually.

## Basic Usage

```python
import uc2rest

# Connect to UC2 device
ESP32 = uc2rest.UC2Client(serialport="/dev/cu.SLAB_USBtoUART", baudrate=115200)

# Home X and Y axes simultaneously
ESP32.home.home_xy(
    axes=["X", "Y"],
    speeds=[15000, 15000],
    directions=[1, -1],
    timeouts=[200, 400],
    isBlocking=True
)
```

## Function Parameters

- **axes**: List of axes to home (e.g., `["X", "Y"]` or `[1, 2]`)
- **speeds**: List of speeds for each axis or single speed for all
- **directions**: List of directions (-1 or 1) for each axis or single direction for all
- **endposreleases**: List of endpos release values or single value for all
- **endstoppolarities**: List of endstop polarities or single value for all  
- **timeouts**: List of timeouts (ms) for each axis or single timeout for all
- **isBlocking**: If True, wait for all motors to finish homing
- **preMove**: If True, move away from endstop before homing

## Generated JSON Command

The function generates a command like this:

```json
{
    "task": "/home_act", 
    "home": {
        "steppers": [
            {"stepperid": 2, "timeout": 400, "speed": 15000, "direction": -1, "endstoppolarity": 1},
            {"stepperid": 1, "timeout": 200, "speed": 15000, "direction": 1, "endstoppolarity": 1}
        ]
    }
}
```

## Axis Mapping

- **"X"** or **1**: X-axis motor
- **"Y"** or **2**: Y-axis motor  
- **"Z"** or **3**: Z-axis motor
- **"A"** or **0**: A-axis motor

## Response Handling

The function expects multiple responses:
1. One acknowledgment response when the command is received
2. One response per motor when each motor completes homing

## Callbacks

You can register a callback to monitor homing status:

```python
def home_callback(is_homed):
    motor_names = ["A", "X", "Y", "Z"]
    for i, homed in enumerate(is_homed):
        status = "HOMED" if homed else "NOT HOMED"
        print(f"Motor {motor_names[i]}: {status}")

ESP32.home.register_callback(0, home_callback)
```

## Examples

### Home X and Y with different parameters
```python
ESP32.home.home_xy(
    axes=["X", "Y"],
    speeds=[15000, 12000],  # Different speeds
    directions=[1, -1],     # X positive, Y negative
    timeouts=[200, 400]     # Different timeouts
)
```

### Home all axes with same parameters
```python
ESP32.home.home_xy(
    axes=["X", "Y", "Z"],
    speeds=10000,          # Same speed for all
    directions=-1,         # All negative direction
    timeouts=5000          # Same timeout for all
)
```

### Using stepper IDs directly
```python
ESP32.home.home_xy(
    axes=[1, 2],           # Stepper IDs 1 and 2
    speeds=[15000, 15000],
    directions=[1, -1],
    timeouts=[200, 400]
)
```

## Advantages

- **Efficiency**: Home multiple motors in parallel instead of sequentially
- **Time saving**: Reduces total homing time significantly
- **Flexibility**: Different parameters for each motor
- **Consistency**: Uses internal parameter defaults when not specified