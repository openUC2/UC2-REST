<p align="center">
  <img src="https://raw.githubusercontent.com/bionanoimaging/UC2-GIT/master/IMAGES/UC2_logo_text.png" width="320" alt="UC2 logo">
</p>

# UC2‑Python Client

Python interface to the **UC2 REST** micro‑controller firmware — control motors, lasers, LED matrices, galvos and more over USB‑Serial or Wi‑Fi from any Python environment.

## Highlights

- **Plug‑and‑play connection** via USB (`/dev/ttyUSB*`, `COM*`) or TCP (`host`, `port`).
- **Rich device model**: every hardware block is a Python object with high‑level helpers (e.g. `motor.move_x`, `led.setPattern`, `laser.set_laser`).
- **Asynchronous & blocking modes** for precise timing or maximum throughput.
- **Callback hooks** to react to hardware feedback in real time.
- **Runs everywhere**: desktop Python, headless Raspberry Pi, or in‑browser with PyScript.
- **LGPL‑3.0‑or‑later** license – use it in academic and commercial projects.

## Installation

```bash
pip install uc2rest           # latest release
# or for the bleeding‑edge version
pip install git+https://github.com/openUC2/UC2-REST.git#subdirectory=PYTHON
```

Dependencies (`requests`, `numpy`, `pyserial`) are resolved automatically.

## Quick start

```python
import uc2rest

# USB example
esp = uc2rest.UC2Client(serialport="/dev/ttyUSB0")         # Linux / Mac OS
# esp = uc2rest.UC2Client(serialport="COM3")               # Windows
# Wi‑Fi example
# esp = uc2rest.UC2Client(host="192.168.4.1", port=31950)

# LED matrix: switch all pixels on at half intensity
esp.led.setAll(True, intensity=128)

# Move X axis by 1 mm (1000 steps @ 1 kHz)
esp.motor.move_x(steps=1000, speed=1000, is_blocking=True)

# Turn green laser to full power
esp.laser.set_laser(channel="G", value=255)
```

## Supported modules (automatically attached to `UC2Client`) citeturn1file12

| Object          | Purpose                               | Example method(s)                    |
|-----------------|---------------------------------------|--------------------------------------|
| `motor`         | X/Y/Z/θ stages                        | `move_x / move_xy`, `setup_motor`    |
| `led`           | 8×8 RGB LED matrix                    | `setPattern`, `send_LEDMatrix_rings` |
| `laser`         | 3‑channel RGB or IR lasers            | `set_laser`, `set_servo`             |
| `galvo`         | Analogue DAC & 2‑axis scanner         | `set_dac`, `set_scanner_pattern`     |
| `gripper`       | Micro‑gripper (servo)                 | `open`, `close`                      |
| `home`          | End‑stop homing routines              | `home_x`, `home_z`                   |
| `rotator`       | Filter wheel or Dove prism            | `move`, `set_speed`                  |
| `objective`     | Piezo objective positioner            | `move_z`, `calibrate`                |
| `temperature`   | NTC digital temperature sensor        | `get_temperature`                    |
| `analog`        | Arbitrary analogue outputs            | `set_voltage`                        |
| `digitalout`    | GPIO (TTL) outputs                    | `set_pin`, `pulse`                   |
| `wifi`          | ESP32 network helpers                 | `scan`, `connect`                    |
| `message`       | Generic key‑value messaging/trigger   | `register_callback`, `trigger_message` |

## Design philosophy

UC2 REST splits interaction into three JSON endpoints per device:

```
/*_act   → perform an action   (e.g. move)
/*_set   → configure a device  (e.g. speed)
/*_get   → query state         (e.g. position)
```

The Python client wraps those endpoints so you rarely deal with raw JSON.


## Running in a browser (PyScript)

```html
<py-config>
  packages = ["uc2rest"]
</py-config>
<py-script>
  from uc2rest import UC2Client
  esp = UC2Client(SerialManager=pyserial_manager)  # see docs
  esp.led.setAll(True)
</py-script>
```

## Documentation & examples

* Jupyter notebook tutorial: `PYTHON/UC2_REST_Tutorial_v0.ipynb`  
* Example scripts: `examples/` directory (motor scan, laser modulation, timelapse)  
* Firmware sources and hardware guides: <https://github.com/openUC2>

## Troubleshooting

| Symptom                      | Fix |
|------------------------------|-----|
| `serial.serialutil.SerialException` | Check port name and permissions (`sudo adduser $USER dialout`) |
| No JSON response / timeout   | Increase `timeout` argument; verify baud rate matches firmware |

## Contributing

1. Fork the repo and create a feature branch.  
2. Follow the `pre‑commit` checks (`black`, `isort`, `flake8`).  
3. Create pull requests against `develop`.

Please open issues for bugs or feature requests.

## License

MIT — see `LICENSE` for details.

---

Made with 💚 by the OpenUC2 community.