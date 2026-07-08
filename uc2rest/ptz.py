"""PTZ keyboard interface for the UC2 CAN PTZ-bridge slave.

A low-cost CCTV PTZ joystick keyboard (Pelco-D/P over RS-485) is read by a
dedicated CAN bridge node (XIAO, default CAN node-id 61). The bridge maps the
joystick to motor moves directly on the bus, and forwards every DISCRETE key
(presets, AUX, iris) to the master, which prints an asynchronous frame:

    {"ptz":{"event":1,"node":61,"type":6,"name":"iris_open","arg":0,"seq":21}}

    type/name:  1 preset_call   2 preset_set   3 preset_clear
                4 aux_on         5 aux_off      6 iris_open
                7 iris_close     8 camera_onoff 9 autoscan
    arg:        preset number (1..255) or AUX number (1..6); 0 otherwise
    seq:        wraps 0..255, one per key event (the master de-dupes on it)

This mirrors the collision "gpio" event plumbing (see gpio.py): a single
serial callback on the top-level "ptz" key, an "event":1 marker to tell a
pushed key from a /ptz_get response, and a list of user callbacks. ImSwitch
registers one such callback and maps each key to a microscope function
(snap image, switch objective, toggle an LED …) — see UC2ConfigController.

Debug / config endpoints (handled locally on the bridge node):
    {"task":"/ptz_act","debug":2}        raw RS-485 hex dump (wiring bring-up)
    {"task":"/ptz_act","debug":1}        decoded frame JSON
    {"task":"/ptz_act","addr":3}         accept only camera address 3 (0=any)
    {"task":"/ptz_act","timeout":2000}   motion watchdog ms (0=off)
    {"task":"/ptz_get"}                  parser stats + last frame + motion
"""


class PTZ(object):
    def __init__(self, parent):
        self._parent = parent

        # last known state (merged from /ptz_get responses and async events)
        self.status = {}
        # last discrete key event pushed by the bridge
        self.last_event = {}

        # register a callback for any {"ptz":{...}} frame on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_ptz, pattern="ptz")

        # user-registered callbacks
        self._event_callbacks = []    # fired on async key events (event==1)
        self._status_callbacks = []   # fired on every ptz frame

    # ─────────────────────────────────────────────────────────────────
    # Async event plumbing
    # ─────────────────────────────────────────────────────────────────
    def _callback_ptz(self, data):
        """Handle any serial frame with a top-level "ptz" key.

        Pushed key events carry "event":1 and fire the event callbacks;
        /ptz_get responses only refresh the cached status."""
        try:
            ptz = data.get("ptz", None)
            if not isinstance(ptz, dict):
                return
            self.status.update(ptz)

            for cb in self._status_callbacks:
                try:
                    cb(dict(self.status))
                except Exception as e:
                    print(f"Error in ptz status callback: {e}")

            if ptz.get("event", 0):
                self.last_event = dict(ptz)
                for cb in self._event_callbacks:
                    try:
                        cb(dict(ptz))
                    except Exception as e:
                        print(f"Error in ptz event callback: {e}")
        except Exception as e:
            print("Error in _callback_ptz: ", e)

    def register_event_callback(self, callbackfct):
        """Register a function called on every asynchronously pushed PTZ key
        event (preset call/set/clear, AUX on/off, iris, …). The callback
        receives the event dict, e.g.
        {"event":1,"node":61,"type":1,"name":"preset_call","arg":1,"seq":7}.
        Use ["name"] and ["arg"] to decide which microscope function to run."""
        if callbackfct not in self._event_callbacks:
            self._event_callbacks.append(callbackfct)

    def register_status_callback(self, callbackfct):
        """Register a function called on EVERY ptz frame (async key events and
        /ptz_get responses alike) with the merged status dict."""
        if callbackfct not in self._status_callbacks:
            self._status_callbacks.append(callbackfct)

    # ─────────────────────────────────────────────────────────────────
    # Query / configuration
    # ─────────────────────────────────────────────────────────────────
    @staticmethod
    def _extract_ptz(response):
        """Unwrap a post_json return into the "ptz" dict.

        mserial.sendMessage returns a LIST of response dicts on success, a
        plain string on timeout, or an int qid — handle all shapes (same
        defensive pattern as GPIO._extract_gpio)."""
        candidates = response if isinstance(response, list) else [response]
        for item in candidates:
            if isinstance(item, dict):
                ptz = item.get("ptz")
                if isinstance(ptz, dict):
                    return ptz
        return {}

    def get_status(self, node=None, timeout=1):
        """Poll the bridge: parser stats (bytesIn/framesOk/resyncs), the last
        decoded frame and the current motion snapshot. Returns the "ptz" dict.
        Note: discrete key events are PUSHED (see register_event_callback);
        polling is only for diagnostics."""
        path = "/ptz_get"
        payload = {"task": path}
        if node is not None:
            payload["node"] = node
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        ptz = self._extract_ptz(r)
        if ptz:
            self.status.update(ptz)
            return ptz
        return dict(self.status)

    def _act(self, node=None, timeout=1, **fields):
        path = "/ptz_act"
        payload = {"task": path}
        if node is not None:
            payload["node"] = node
        payload.update(fields)
        return self._parent.post_json(path, payload, getReturn=True, timeout=timeout)

    def set_debug(self, level, node=None, timeout=1):
        """Bridge serial debug verbosity: 0 quiet, 1 decoded frame JSON,
        2 + raw RS-485 hex dump (use for wiring / baud / A-B-swap bring-up)."""
        return self._act(node=node, timeout=timeout, debug=int(level))

    def set_address(self, addr, node=None, timeout=1):
        """Only accept keyboard frames for this Pelco camera address
        (0 = accept any). Handy when several bridges share one keyboard."""
        return self._act(node=node, timeout=timeout, addr=int(addr))

    def set_motion_timeout(self, timeout_ms, node=None, serial_timeout=1):
        """Watchdog (ms): stop all axes if the keyboard goes silent while an
        axis is moving (0 = off). Guards against a lost stop frame.

        Built explicitly (not via _act) because the firmware field is itself
        named "timeout", which would clash with the serial-call timeout kwarg."""
        path = "/ptz_act"
        payload = {"task": path, "timeout": int(timeout_ms)}
        if node is not None:
            payload["node"] = node
        return self._parent.post_json(path, payload, getReturn=True,
                                      timeout=serial_timeout)
