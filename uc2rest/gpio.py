"""GPIO / collision-detector interface for the UC2 CAN GPIO slave.

The CAN master exposes the GPIO slave (XIAO node, default CAN node-id 60)
through two serial endpoints:

    {"task":"/gpio_get"}
        -> {"gpio":{"node":60,"mean":585,"filtered":584,"raw":581,
                    "reference":585,"threshold":150,"sensitivity":4,
                    "trip":0,"estop":0},"ok":true,"qid":N}

    {"task":"/gpio_act","threshold":150,"sensitivity":4,
     "reference":585,"calibrate":1}          (all value fields optional)

Collision detection runs ON THE SLAVE: the sensor has an idle value (the
"reference"); a collision = `sensitivity` consecutive samples deviating more
than `threshold` ADC counts from the reference (rising OR falling). Sensor
values are never streamed on the CAN bus — polling /gpio_get triggers SDO
reads on demand. Only the collision/E-stop EVENT is pushed asynchronously;
the master then prints:

    {"gpio":{"event":1,"node":60,"trip":1,"estop":0,"flags":3,
             "filtered":2365,"raw":2937,...}}

The "event":1 marker distinguishes pushed edges from /gpio_get responses
(both share the top-level "gpio" key and therefore both arrive at the same
serial callback).
"""


class GPIO(object):
    def __init__(self, parent):
        self._parent = parent

        # last known state (merged from get_status responses and async events)
        self.status = {}

        # register a callback for any {"gpio":{...}} frame on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_gpio, pattern="gpio")

        # user-registered callbacks
        self._collision_callbacks = []   # fired on async trip/clear events
        self._status_callbacks = []      # fired on every gpio frame

    # ─────────────────────────────────────────────────────────────────
    # Async event plumbing
    # ─────────────────────────────────────────────────────────────────
    def _callback_gpio(self, data):
        """Handle any serial frame with a top-level "gpio" key.

        Pushed events carry "event":1 and fire the collision callbacks;
        /gpio_get responses only refresh the cached status."""
        try:
            gpio = data.get("gpio", None)
            if not isinstance(gpio, dict):
                return
            self.status.update(gpio)

            for cb in self._status_callbacks:
                try:
                    cb(dict(self.status))
                except Exception as e:
                    print(f"Error in gpio status callback: {e}")

            if gpio.get("event", 0):
                for cb in self._collision_callbacks:
                    try:
                        cb(dict(gpio))
                    except Exception as e:
                        print(f"Error in gpio collision callback: {e}")
        except Exception as e:
            print("Error in _callback_gpio: ", e)

    def register_collision_callback(self, callbackfct):
        """Register a function called on every asynchronously pushed GPIO
        event (collision trip/clear, E-stop press/release). The callback
        receives the event dict, e.g. {"event":1,"trip":1,"estop":0,...}.
        Check ["trip"] to distinguish collision-began from collision-cleared."""
        if callbackfct not in self._collision_callbacks:
            self._collision_callbacks.append(callbackfct)

    def register_status_callback(self, callbackfct):
        """Register a function called on EVERY gpio frame (async events and
        /gpio_get responses alike) with the merged status dict."""
        if callbackfct not in self._status_callbacks:
            self._status_callbacks.append(callbackfct)

    # ─────────────────────────────────────────────────────────────────
    # Query / configuration
    # ─────────────────────────────────────────────────────────────────
    @staticmethod
    def _extract_gpio(response):
        """Unwrap a post_json return into the "gpio" dict.

        mserial.sendMessage returns a LIST of response dicts on success
        (self.responses[qid] is a per-frame list), a plain string on
        timeout/interrupt, or an int qid when getReturn=False — so handle
        all shapes defensively."""
        candidates = response if isinstance(response, list) else [response]
        for item in candidates:
            if isinstance(item, dict):
                gpio = item.get("gpio")
                if isinstance(gpio, dict):
                    return gpio
        return {}

    def get_status(self, node=None, timeout=1):
        """Poll the collision detector (SDO reads on the CAN bus).

        Returns the "gpio" dict: mean (rolling average), filtered, raw,
        baseline, sigma, mode (0=auto/1=manual), reference, threshold,
        sensitivity, trip, estop."""
        path = "/gpio_get"
        payload = {"task": path}
        if node is not None:
            payload["node"] = node
        r = self._parent.post_json(path, payload, getReturn=True, timeout=timeout)
        gpio = self._extract_gpio(r)
        if gpio:
            self.status.update(gpio)
            return gpio
        # Response didn't parse (timeout/odd shape) — fall back to the cache
        # maintained by the async serial callback, which sees every frame.
        return dict(self.status)

    def _act(self, node=None, timeout=1, **fields):
        path = "/gpio_act"
        payload = {"task": path}
        if node is not None:
            payload["node"] = node
        payload.update(fields)
        return self._parent.post_json(path, payload, getReturn=True, timeout=timeout)

    def set_threshold(self, threshold, node=None, timeout=1):
        """Deviation band (ADC counts) around the reference outside which a
        sample votes for "collision". Persisted in the slave's NVS."""
        return self._act(node=node, timeout=timeout, threshold=int(threshold))

    def set_sensitivity(self, sensitivity, node=None, timeout=1):
        """Number of CONSECUTIVE out-of-band samples (at 50 Hz) required to
        trip — and in-band samples to clear. 3-4 rejects single-sample spikes
        while confirming a real collision within ~60-80 ms."""
        return self._act(node=node, timeout=timeout, sensitivity=int(sensitivity))

    def set_reference(self, reference, node=None, timeout=1):
        """Explicitly set the idle baseline (ADC counts). Typically you poll
        get_status()["mean"] while the system is idle and write that value
        here — or simply use calibrate()."""
        return self._act(node=node, timeout=timeout, reference=int(reference))

    def calibrate(self, node=None, timeout=1):
        """Tell the slave to take its CURRENT rolling mean as the new
        reference (persisted in NVS). Do this while the system is idle and
        collision-free."""
        return self._act(node=node, timeout=timeout, calibrate=1)
