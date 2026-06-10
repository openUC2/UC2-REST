class State(object):
    '''
    ##############################################################################################################################
    STATE
    ##############################################################################################################################
    '''

    def __init__(self, parent):
        self._parent = parent

        # Emergency-STOP event handling. The CANopen master firmware emits an
        # unsolicited block when the hardware E-stop is pressed/released:
        #   ++
        #   {"emergency":{"active":1,"reason":"estop","msg":"..."},"qid":0}
        #   --
        # We surface it through registered callbacks (see register_emergency_callback).
        self.emergency_active = False
        self.last_emergency = None
        self._emergency_callbacks = []
        if hasattr(self._parent, "serial") and hasattr(self._parent.serial, "register_callback"):
            self._parent.serial.register_callback(self._callback_emergency, pattern="emergency")

    # ──────────────────────────────────────────────────────────────────────
    # CAN-bus power (CANopen master gate driven by BUSPOWER_OFF_PIN)
    # ──────────────────────────────────────────────────────────────────────
    def set_power(self, power=1, getReturn=True, timeout=1):
        '''
        Enable (1, default) or disable (0) the high-current CAN-bus power that
        feeds the slaves. Maps to /state_act {"power":0|1} on the master.
        '''
        path = "/state_act"
        payload = {
            "task": path,
            "power": int(bool(power))
        }
        return self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)

    def get_power(self, timeout=1):
        '''
        Query the current CAN-bus power state (1=ON, 0=OFF).
        Maps to /state_get {"power":1}. Returns an int when parseable,
        otherwise the raw response.
        '''
        path = "/state_get"
        payload = {
            "task": path,
            "power": 1
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try:
            if isinstance(r, list):
                r = r[0]
            return r["state"]["power"]
        except Exception:
            return r

    # ──────────────────────────────────────────────────────────────────────
    # Emergency-STOP (async event pushed by the firmware)
    # ──────────────────────────────────────────────────────────────────────
    def set_estop_polarity(self, polarity=0, getReturn=True, timeout=1):
        '''
        Set the emergency-STOP asserted (emergency) logic level. Persisted on the
        device and applied live.
          polarity=0 -> active-LOW  (asserted reads LOW,  idle HIGH)
          polarity=1 -> active-HIGH (asserted reads HIGH, idle LOW)
        Maps to /state_act {"estopPolarity":0|1}.
        '''
        path = "/state_act"
        payload = {"task": path, "estopPolarity": int(bool(polarity))}
        return self._parent.post_json(path, payload, getReturn=getReturn, timeout=timeout)

    def get_estop(self, timeout=1):
        '''
        Read E-stop diagnostics as a dict:
        {"estopPolarity":0|1, "estopRaw":0|1|-1, "estopActive":0|1}.
        estopRaw is the live pin level — flip the polarity (set_estop_polarity)
        until pressing the button makes estopActive read 1.
        Maps to /state_get {"estop":1}.
        '''
        path = "/state_get"
        payload = {"task": path, "estop": 1}
        r = self._parent.post_json(path, payload, timeout=timeout)
        try:
            if isinstance(r, list):
                r = r[0]
            return r["state"]
        except Exception:
            return r

    def _callback_emergency(self, data):
        ''' Handle the async {"emergency":{...}} event from the firmware. '''
        try:
            em = data.get("emergency", {})
            self.emergency_active = bool(em.get("active", 0))
            self.last_emergency = em
            if self.emergency_active:
                self._parent.logger.warning(
                    "EMERGENCY STOP: " + str(em.get("msg", "E-stop asserted")))
            else:
                self._parent.logger.info("Emergency stop released")
            for callback in list(self._emergency_callbacks):
                try:
                    callback(em)
                except Exception as callback_error:
                    self._parent.logger.error(
                        "Error in emergency callback execution: " + str(callback_error))
        except Exception as e:
            self._parent.logger.error("Error in _callback_emergency: " + str(e))

    def register_emergency_callback(self, callbackfct):
        '''
        Register a function invoked whenever an emergency event arrives.
        The callback receives the "emergency" dict, e.g.
        {"active":1,"reason":"estop","msg":"..."}.

        Example:
            def on_estop(em):
                if em.get("active"):
                    print("E-STOP! stopping host-side acquisition")
            ESP32.state.register_emergency_callback(on_estop)
        '''
        if callbackfct not in self._emergency_callbacks:
            self._emergency_callbacks.append(callbackfct)

    def unregister_emergency_callback(self, callbackfct):
        ''' Remove a previously registered emergency callback. '''
        if callbackfct in self._emergency_callbacks:
            self._emergency_callbacks.remove(callbackfct)

    def is_emergency_active(self):
        ''' Last known emergency state as reported by the firmware. '''
        return self.emergency_active

    def get_state(self, timeout=3):
        path = "/state_get"

        r = self._parent.get_json(path, timeout=timeout)
        return r

    def delay(self, delay=1, getReturn=True):
        path = "/state_act"
        payload = {
            "task":path,
            "delay":delay
        }
        r = self._parent.post_json(path, payload, getReturn=getReturn)
        return r

    def set_state(self, debug=False, timeout=1):
        path = "/state_set"

        payload = {
            "task":path,
            "isdebug":int(debug)
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def isControllerMode(self, timeout=1):
        # returns True if PS controller is active
        path = "/state_get"
        payload = {
            "task":path,
            "pscontroller": 1
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try:
            return r["pscontroller"]
        except:
            return False
        
    def pairBT(self, timeout=1):
        path = "/bt_scan"
        payload={
            "connect": 1}
        r = self._parent.post_json(path, payload, getReturn=False, timeout=timeout)
        return r
    
    def espRestart(self,timeout=1):
        # if isController =True=> only PS jjoystick will be accepted
        # {"task":"/state_act", "restart":1}
        path = "/state_act"
        payload = {
            "restart":1
            }
        r = self._parent.post_json(path, payload, getReturn=False, timeout=timeout)
        return r

    def setControllerMode(self, isController=False, timeout=1):
        # if isController =True=> only PS jjoystick will be accepted
        path = "/state_act"
        payload = {
            "task":path,
            "pscontroller": isController
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        return r

    def isBusy(self, timeout=1):
        path = "/state_get"
        payload = {
            "task":path,
            "isBusy": 1
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try:
            return r["isBusy"]
        except:
            return r


    def getHeap(self, timeout=1):
        path = "/state_get"
        payload = {
            "task":path,
            "heap": 1
        }
        r = self._parent.post_json(path, payload, timeout=timeout)
        try:
            return r["heap"]
        except:
            return r