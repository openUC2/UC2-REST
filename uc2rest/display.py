import unicodedata

# Longest string the firmware keeps per line (hostnames are <= 63 characters).
MAX_TEXT = 63

# What NFKD leaves behind for the characters ImSwitch step names actually use.
_ASCII_FIXES = {"→": ">", "←": "<", "µ": "u", "°": "deg", "–": "-", "—": "-", "×": "x", "…": "...", "ß": "ss"}


def _ascii(text, maxlen=MAX_TEXT):
    """The OLED font is plain ASCII: transliterate what we can, drop the rest."""
    text = "" if text is None else str(text)
    for src, dst in _ASCII_FIXES.items():
        text = text.replace(src, dst)
    text = unicodedata.normalize("NFKD", text).encode("ascii", "ignore").decode("ascii")
    return "".join(c if c.isprintable() else " " for c in text)[:maxlen]


class Display(object):
    """
    Status OLED (0.91" SSD1306, 128x32, I2C) on the CAN-HAT master.

    Firmware endpoints: /display_act and /display_get. The display shows the
    firmware version for 10 s after power-up, then a big title line (the device
    name, "openuc2" until one is set), a status line (e.g. the running
    experiment step) and the firmware version or a progress bar. Text wider than
    the display scrolls on the ESP, so each string only has to be sent once.

        ESP32.display.set_name("openuc2-metal-slope-23501")
        ESP32.display.set_status("Scanning 12/340", progress=3)
        ESP32.display.clear_status()
        ESP32.display.off()
    """

    def __init__(self, parent):
        self._parent = parent
        self._state = {}
        self._fresh = False  # a {"display":...} reply arrived since get_display() started

        # Replies are matched to requests by qid. Firmware that answers /display_get
        # without echoing it (before 2026-11-18) never reaches the blocking read below,
        # so the reply is also picked up here by its key, as Fan does.
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_display, pattern="display")

    def _callback_display(self, data):
        state = data.get("display")
        if isinstance(state, dict):
            self._state = state
            self._fresh = True

    # ── getters ──────────────────────────────────────────────────────────────

    def get_display(self, timeout=1):
        """
        {"task":"/display_get"} →
        {present, on, addr, name, status, progress, fw, flip, contrast, splash}.
        Returns {} when the firmware has no display module / did not answer.
        """
        self._fresh = False
        r = self._parent.get_json("/display_get", timeout=timeout)
        try:
            if isinstance(r, list):
                r = r[0]
            self._state = r["display"]
        except Exception:
            if not self._fresh:  # no qid-less reply either: nothing answered
                self._state = {}
        return self._state

    def is_present(self, timeout=1):
        """True when firmware has the display module AND a panel answers on the I2C bus."""
        return bool(self.get_display(timeout=timeout).get("present", 0))

    # ── content ──────────────────────────────────────────────────────────────

    def set_name(self, name):
        """Title line, stored on the ESP (survives a reboot). "" restores "openuc2"."""
        return self._act({"name": _ascii(name)})

    def set_status(self, status, progress=None):
        """Status line (scrolls if longer than 21 characters); optional progress bar 0-100."""
        payload = {"status": _ascii(status)}
        if progress is not None:
            payload["progress"] = self._clamp_progress(progress)
        return self._act(payload)

    def set_progress(self, progress):
        """Progress bar 0-100 in place of the firmware-version line; None/-1 hides it."""
        return self._act({"progress": self._clamp_progress(progress)})

    def clear_status(self):
        """Empty the status line and hide the progress bar."""
        return self._act({"status": "", "progress": -1})

    def show_splash(self):
        """Replay the 10 s boot screen with the firmware version."""
        return self._act({"splash": 1})

    # ── hardware ─────────────────────────────────────────────────────────────

    def set_on(self, on=True):
        """Switch the panel on/off (charge pump too, so it is fully dark). Stored on the ESP."""
        return self._act({"on": 1 if on else 0})

    def on(self):
        return self.set_on(True)

    def off(self):
        return self.set_on(False)

    def set_flip(self, flip=True):
        """Rotate the picture by 180 degrees (for the other mounting orientation). Stored on the ESP."""
        return self._act({"flip": 1 if flip else 0})

    def set_contrast(self, contrast=143):
        """Brightness 0-255 (default 143). Lower values spare the OLED. Stored on the ESP."""
        return self._act({"contrast": int(max(0, min(255, contrast)))})

    # ── internal ─────────────────────────────────────────────────────────────

    @staticmethod
    def _clamp_progress(progress):
        try:
            return int(max(-1, min(100, progress)))
        except (TypeError, ValueError):
            return -1

    def _act(self, display_payload):
        payload = {"task": "/display_act", "display": display_payload}
        return self._parent.post_json("/display_act", payload, getReturn=False)


if __name__ == "__main__":
    import time
    import uc2rest
    port = "/dev/cu.SLAB_USBtoUART"
    ESP32 = uc2rest.UC2Client(serialport=port, DEBUG=True)

    print("display:", ESP32.display.get_display())
    ESP32.display.set_name("openuc2-metal-slope-23501")  # 25 chars: scrolls on the 2x title line
    ESP32.display.set_status("Scanning 12/340", progress=3)
    time.sleep(5)
    ESP32.display.clear_status()
    ESP32.display.off()
    time.sleep(2)
    ESP32.display.on()

    ESP32.close()
