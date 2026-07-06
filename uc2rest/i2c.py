"""Generic raw-I2C passthrough for the UC2 CAN GPIO slave.

Any I2C device wired to the GPIO slave's sensor bus (SDA=GPIO5/D4,
SCL=GPIO6/D5 on the can-gpio board) can be read/written from Python WITHOUT
a device driver on the ESP32. The firmware just relays bytes; the register map
lives here. One JSON request = one raw transaction (optional write, optional
inter-phase delay, optional read):

    {"task":"/i2c_act","node":60,"addr":68,"write":[253],"read":6,
     "delay":12,"stop":1}
        -> {"node":60,"status":2,"data":[.. up to 40 bytes ..],"ok":true}

    addr  : 7-bit I2C address (decimal; 0x44 == 68)
    write : bytes to send first (register pointer / command / config)
    read  : number of bytes to read back (0 = write-only)
    delay : ms to wait between the write and the read (device conversion time)
    stop  : 1 → send STOP after the write (device wants a full stop before the
            read, e.g. SHT4x); 0/omitted → repeated-START (typical register read)

Status byte: 2 = ok, 0x81..0x85 = write failed (I2C NACK / bus error),
0x90 = fewer bytes read than requested, 0xA0 = bad parameters.

See uc2-ESP: main/src/i2c/I2cBridge.cpp and DeviceRouter::handleI2cAct.
"""

import time


class I2C(object):
    STATUS_OK = 0x02

    def __init__(self, parent):
        self._parent = parent

    # ─────────────────────────────────────────────────────────────────
    # Generic transaction
    # ─────────────────────────────────────────────────────────────────
    @staticmethod
    def _extract(response):
        """Unwrap a post_json return into the response dict carrying "data".

        mserial.sendMessage returns a LIST of frames on success, a plain
        string on timeout, or an int qid when getReturn=False — handle all."""
        candidates = response if isinstance(response, list) else [response]
        for item in candidates:
            if isinstance(item, dict) and ("data" in item or "status" in item):
                return item
        return {}

    def transaction(self, addr, write=None, read=0, delay_ms=0, stop=False,
                    node=None, timeout=2):
        """Run one raw I2C transaction on the slave and return the response
        dict {"status":int, "data":[...], ...}. `write` is an iterable of
        byte values; `read` is how many bytes to read back."""
        payload = {"task": "/i2c_act",
                   "addr": int(addr) & 0x7F,
                   "read": int(read),
                   "delay": int(delay_ms)}
        if write:
            payload["write"] = [int(b) & 0xFF for b in write]
        if stop:
            payload["stop"] = 1
        if node is not None:
            payload["node"] = node
        r = self._parent.post_json("/i2c_act", payload, getReturn=True,
                                   timeout=timeout)
        return self._extract(r)

    def ok(self, response):
        """True if a transaction response reports success."""
        return isinstance(response, dict) and response.get("status") == self.STATUS_OK

    # ─────────────────────────────────────────────────────────────────
    # Register-level convenience wrappers
    # ─────────────────────────────────────────────────────────────────
    def read_register(self, addr, reg, length, node=None,
                      repeated_start=True, timeout=2):
        """Classic register read: write the register pointer, then read
        `length` bytes (repeated-START by default). Returns a bytes object
        (empty on failure)."""
        res = self.transaction(addr, write=[reg], read=length,
                               stop=not repeated_start, node=node, timeout=timeout)
        return bytes(res.get("data", []) or [])

    def write_register(self, addr, reg, values, node=None, timeout=2):
        """Write `values` (int or iterable) to register `reg`. Returns True on
        an acknowledged write."""
        if isinstance(values, int):
            values = [values]
        res = self.transaction(addr, write=[reg] + [int(v) & 0xFF for v in values],
                               read=0, stop=True, node=node, timeout=timeout)
        return self.ok(res)

    def scan(self, node=None, start=0x08, end=0x77, timeout=2):
        """Probe the bus for responding devices (0-byte write per address).
        Returns a list of 7-bit addresses that ACKed."""
        found = []
        for a in range(start, end + 1):
            # zero-length write: beginTransmission + endTransmission(true).
            res = self.transaction(a, write=[], read=0, stop=True,
                                   node=node, timeout=timeout)
            if self.ok(res):
                found.append(a)
        return found

    # ─────────────────────────────────────────────────────────────────
    # Adafruit SHT45 — humidity + temperature (addr 0x44)
    # ─────────────────────────────────────────────────────────────────
    @staticmethod
    def _sht_crc(data):
        """Sensirion CRC-8: poly 0x31, init 0xFF."""
        crc = 0xFF
        for b in data:
            crc ^= b
            for _ in range(8):
                crc = ((crc << 1) ^ 0x31) & 0xFF if (crc & 0x80) else (crc << 1) & 0xFF
        return crc

    def read_sht45(self, addr=0x44, node=None, precision="high", timeout=2):
        """Read the SHT45. Sends the measure command, waits for the
        conversion, reads 6 bytes (T, T_crc, RH, RH_crc) and converts.

        precision: "high" (0xFD, ~9 ms), "med" (0xF6), "low" (0xE0).
        Returns {"temperature_c", "humidity_pct", "crc_ok"} or None on error."""
        cmd = {"high": 0xFD, "med": 0xF6, "low": 0xE0}.get(precision, 0xFD)
        res = self.transaction(addr, write=[cmd], read=6, delay_ms=12,
                               stop=True, node=node, timeout=timeout)
        data = res.get("data", []) or []
        if len(data) < 6:
            return None
        t_ticks = (data[0] << 8) | data[1]
        rh_ticks = (data[3] << 8) | data[4]
        temp_c = -45.0 + 175.0 * t_ticks / 65535.0
        rh = -6.0 + 125.0 * rh_ticks / 65535.0
        rh = max(0.0, min(100.0, rh))
        crc_ok = (self._sht_crc(data[0:2]) == data[2] and
                  self._sht_crc(data[3:5]) == data[5])
        return {"temperature_c": temp_c, "humidity_pct": rh, "crc_ok": crc_ok}

    # ─────────────────────────────────────────────────────────────────
    # Adafruit TSL2591 — high-dynamic-range light sensor (addr 0x29)
    # ─────────────────────────────────────────────────────────────────
    # Command byte: 0xA0 = normal register access; low 5 bits = register addr.
    _TSL_CMD = 0xA0
    _TSL_ENABLE = 0x00     # PON(0x01) | AEN(0x02) | (AIEN/NPIEN optional)
    _TSL_CONTROL = 0x01    # AGAIN[5:4] | ATIME[2:0]
    _TSL_C0DATAL = 0x14    # CH0 low; auto-increments through 0x17 (CH1 high)
    _TSL_GAIN = {0x00: 1.0, 0x10: 25.0, 0x20: 428.0, 0x30: 9876.0}

    def read_tsl2591(self, addr=0x29, node=None, gain=0x10, integration=0x01,
                     timeout=2):
        """Read the TSL2591 broadband (CH0) and IR (CH1) channels and estimate
        lux. Powers the ALS on, applies gain/integration, waits one integration
        period, then reads the four data bytes.

        gain:        0x00=1x, 0x10=25x (default), 0x20=428x, 0x30=9876x
        integration: 0x00=100ms .. 0x05=600ms (default 0x01 = 200ms)
        Returns {"ch0_full","ch1_ir","visible","gain","integration_ms",
                 "lux","saturated"} or None on error."""
        # Enable ALS (PON | AEN).
        if not self.write_register(addr, self._TSL_CMD | self._TSL_ENABLE, 0x03,
                                   node=node, timeout=timeout):
            return None
        # Set gain + integration time.
        self.write_register(addr, self._TSL_CMD | self._TSL_CONTROL,
                            (gain & 0x30) | (integration & 0x07),
                            node=node, timeout=timeout)
        atime_ms = 100 * ((integration & 0x07) + 1)
        # Wait one full integration period (Python-side so we aren't limited to
        # the firmware's 255 ms per-transaction delay cap). Small margin added.
        time.sleep(atime_ms / 1000.0 + 0.05)
        # Burst-read CH0/CH1 (4 bytes, auto-increment) via repeated-START.
        res = self.transaction(addr, write=[self._TSL_CMD | self._TSL_C0DATAL],
                               read=4, stop=False, node=node, timeout=timeout)
        data = res.get("data", []) or []
        if len(data) < 4:
            return None
        ch0 = data[0] | (data[1] << 8)   # full spectrum
        ch1 = data[2] | (data[3] << 8)   # IR
        saturated = ch0 >= 0xFFFF or ch1 >= 0xFFFF

        # Approximate lux (standard TSL2591 formula; device lot variation ~±).
        again = self._TSL_GAIN.get(gain & 0x30, 1.0)
        lux = None
        if ch0 > 0 and not saturated:
            cpl = (atime_ms * again) / 408.0   # counts-per-lux, DF = 408
            lux = max(0.0, (ch0 - ch1) * (1.0 - float(ch1) / float(ch0)) / cpl)
        return {"ch0_full": ch0, "ch1_ir": ch1, "visible": ch0 - ch1,
                "gain": gain, "integration_ms": atime_ms,
                "lux": lux, "saturated": saturated}
