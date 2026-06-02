import time

class Fan(object):

    def __init__(self, parent):
        self._parent = parent
        self._fan_state = {}
        self._temp_state = {}

        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_fan, pattern="fan")

    def _callback_fan(self, data):
        try:
            self._fan_state = data["fan"]
        except Exception as e:
            pass

    # ── getters ──────────────────────────────────────────────────────────────

    def get_fan(self, timeout=1, blocking=True):
        """{"task":"/fan_get"} → {fan:{mode,wiper,manual,rpm,stalled,kick,tempC,curve}}"""
        if blocking:
            r = self._parent.get_json("/fan_get", timeout=timeout)
            try:
                self._fan_state = r[0]["fan"]
            except Exception:
                pass
            return self._fan_state
        return self._fan_state

    def get_temp(self, timeout=1):
        """{"task":"/temp_get"} → {temp:{pcb,air,esp,pcb_ok,air_ok}}"""
        r = self._parent.get_json("/temp_get", timeout=timeout)
        try:
            self._temp_state = r[0]["temp"]
        except Exception:
            pass
        return self._temp_state

    # ── setters ───────────────────────────────────────────────────────────────

    def set_mode(self, mode="auto", wiper=None):
        """mode: 'auto' | 'manual' | 'off'. wiper 0-127 required for manual."""
        fan = {"mode": mode}
        if wiper is not None:
            fan["wiper"] = int(max(0, min(127, wiper)))
        return self._act(fan)

    def set_wiper(self, wiper):
        """Set fan wiper directly (implies manual mode), wiper 0-127."""
        return self._act({"mode": "manual", "wiper": int(max(0, min(127, wiper)))})

    def set_kick(self, enabled):
        """Enable (True/1) or disable (False/0) stall-kick-start."""
        return self._act({"kick": 1 if enabled else 0})

    def set_curve(self, curve):
        """curve: list of [thresholdC, wiper] pairs, e.g. [[35,127],[45,80]]"""
        return self._act({"curve": curve})

    def off(self):
        return self._act({"mode": "off"})

    # ── internal ──────────────────────────────────────────────────────────────

    def _act(self, fan_payload):
        payload = {"task": "/fan_act", "fan": fan_payload}
        return self._parent.post_json("/fan_act", payload, getReturn=False)


if __name__ == "__main__":
    import uc2rest
    port = "/dev/cu.SLAB_USBtoUART"
    ESP32 = uc2rest.UC2Client(serialport=port, DEBUG=True)

    print("temp:", ESP32.fan.get_temp())
    print("fan:", ESP32.fan.get_fan())

    ESP32.fan.set_mode("auto")
    time.sleep(1)
    ESP32.fan.set_wiper(48)
    time.sleep(1)
    ESP32.fan.set_mode("off")

    ESP32.close()
