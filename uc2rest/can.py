class CAN(object):
    def __init__(self, parent):
        """
        CANController handles sending commands to a remote CAN device via the parent post_json interface.

        :param parent: Parent object with post_json(path, payload, getReturn, timeout, nResponses)
        """
        self._parent = parent
        
        # Store latest scan results
        self.scanResults = []
        self.deviceCount = 0
        
        # Register a callback function for the CAN status on the serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_can_status, pattern="scan")
        
        # Announce a function that is called when we receive a CAN scan update through the callback
        self._callbackPerKey = {}
        self.nCallbacks = 10
        self._callbackPerKey = self.init_callback_functions(nCallbacks=self.nCallbacks)
        print(self._callbackPerKey)
    
    def init_callback_functions(self, nCallbacks=10):
        """Initialize the callback functions."""
        _callbackPerKey = {}
        self.nCallbacks = nCallbacks
        for i in range(nCallbacks):
            _callbackPerKey[i] = []
        return _callbackPerKey
    
    def _callback_can_status(self, data):
        """
        Cast the json in the form:
        {
            "scan": [
                {"canId": 20, "deviceType": 1, "status": 0, "deviceTypeStr": "laser", "statusStr": "idle"},
                {"canId": 10, "deviceType": 0, "status": 0, "deviceTypeStr": "motor", "statusStr": "idle"}
            ],
            "qid": 2,
            "count": 1
        }
        into the scan results array.
        """
        try:
            if "scan" in data:
                self.scanResults = data["scan"]
                self.deviceCount = data.get("count", len(self.scanResults))
                
                # Call registered callback function with scan results
                if callable(self._callbackPerKey[0]):
                    self._callbackPerKey[0](self.scanResults)
        except Exception as e:
            print("Error in _callback_can_status: ", e)
    
    def register_callback(self, key, callbackfct):
        """Register a callback function for a specific key."""
        self._callbackPerKey[key] = callbackfct

    def get_can_ids(self):
        """Return CAN node IDs from the latest scan (e.g. for OTA flashing)."""
        return [entry.get("canId") for entry in self.scanResults
                if entry.get("canId") is not None]

    def reboot_remote(self, qid=1, can_address=0, isBlocking=False, timeout=2):
        """
        Reboot a CAN device.

        - ``can_address == 0`` reboots the master (this ESP32) itself.
        - ``can_address in 1..127`` reboots a remote slave by SDO-writing 1
          to OD index 0x2507 sub 0 on the target node. The slave's
          CO_tmr_task observes the write and calls ESP.restart() ~200 ms
          later.

        :param qid: Query ID for the CAN command (unused by firmware, kept
            for API compatibility)
        :param can_address: 0 = master, 1..127 = remote slave nodeId
        :param isBlocking: If True, wait for response
        :param timeout: Timeout for the command in seconds
        :return: Response from the device, e.g.
            ``{"status":"ok","nodeId":11}`` or
            ``{"status":"error","error":"SDO write failed","nodeId":11}``.
        """
        path = "/can_act"
        payload = {
            "task": path,
            "restart": int(can_address)
        }
        nResponses = 1 if isBlocking else 0
        # Send the payload to the parent, which handles the actual communication
        return self._parent.post_json(
            path,
            payload,
            getReturn=isBlocking,
            timeout=timeout if isBlocking else 0,
            nResponses=nResponses
        )

    def scan(self, qid=1, timeout=5, probe_range=False, id_from=1, id_to=127):
        """
        Scan the CAN bus for connected devices.

        :param qid: Query ID for the CAN command (default: 1)
        :param timeout: Timeout for the scan in seconds (default: 5)
        :param probe_range: if True, additionally SDO-probe node-ids
            ``id_from..id_to`` for their MAC so brand-new / unrouted nodes are
            discovered (reported with deviceTypeStr "unrouted"). Absent ids
            fast-fail on the firmware side. Use this for MAC-keyed provisioning.
        :param id_from: first node-id to probe when ``probe_range`` is set
        :param id_to: last node-id to probe when ``probe_range`` is set
        :return: Response containing scan results with device information.
                 Reachable nodes are SDO-probed for their firmware build
                 timestamp (OD 0x2508), version (0x2500) and MAC (0x2509);
                 the master reports its own identity under the "master" key.
                 Example: {
                     "master": {"canId": 1, "build": "Jun 22 2026 14:30:11",
                                "fwVersion": "UC2-ESP v2.0", "mac": "AA:BB:CC:DD:EE:01"},
                     "scan": [
                         {"canId": 10, "deviceType": 0, "deviceTypeStr": "motor",
                          "status": 0, "statusStr": "idle",
                          "build": "Jun 22 2026 14:31:02", "fwVersion": "UC2-ESP v2.0",
                          "mac": "AA:BB:CC:DD:EE:10"},
                         {"canId": 20, "deviceType": 1, "deviceTypeStr": "laser",
                          "status": 1, "statusStr": "unreachable"}
                     ],
                     "qid": 1,
                     "count": 2
                 }
                 Note: "build"/"fwVersion"/"mac" are only present for nodes that
                 answered the SDO probe (i.e. statusStr == "idle").
        """
        path = "/can_act" #         {"task":"/can_act", "scan": true}
        payload = {
            "task": path,
            "scan": True,
            "qid": qid
        }
        if probe_range:
            payload["probeRange"] = True
            payload["from"] = int(id_from)
            payload["to"] = int(id_to)
        return self._parent.post_json(
            path,
            payload,
            getReturn=True,
            timeout=timeout,
            nResponses=1
        )

    def discover(self, qid=1, timeout=8, id_from=1, id_to=127):
        """
        Discover all nodes on the bus by SDO-probing a node-id range.

        Convenience wrapper around ``scan(probe_range=True, ...)`` — finds nodes
        that aren't in the master's routing table yet (e.g. freshly flashed
        boards), reporting their MAC + build info so they can be provisioned by
        MAC via :meth:`assign_node_id_by_mac`.

        :return: same shape as :meth:`scan`; probed-but-unrouted nodes carry
            deviceTypeStr "unrouted".
        """
        return self.scan(qid=qid, timeout=timeout,
                         probe_range=True, id_from=id_from, id_to=id_to)

    def get_available_devices(self, timeout=2):
        """
        Get list of available CAN devices.

        :param timeout: Timeout for the command in seconds (default: 2)
        :return: Response containing available CAN IDs and other bus information
                 Example: {
                     "input": {"task": "/can_get"},
                     "address": 1,
                     "addresspref": 1,
                     "addressgetcan": 1,
                     "nonworking": [0,0,0,...],
                     "available": [20,0,0,...],
                     "rx": 18,
                     "tx": 17
                 }
        """
        path = "/can_act"
        payload = {
            "task": path,
            "scan": True
        }
        return self._parent.post_json(
            path,
            payload,
            getReturn=True,
            timeout=timeout,
            nResponses=2
        )

    def get_device_build_info(self):
        """
        Return per-node firmware build info from the latest scan results.

        :return: dict keyed by CAN id, e.g.
                 {10: {"build": "Jun 22 2026 14:31:02",
                       "fwVersion": "UC2-ESP v2.0",
                       "mac": "AA:BB:CC:DD:EE:10"}}
                 Only nodes that answered the SDO probe during the last scan()
                 appear here. Call scan() first to refresh.
        """
        info = {}
        for entry in self.scanResults:
            cid = entry.get("canId")
            if cid is None:
                continue
            fields = {k: entry[k] for k in ("build", "fwVersion", "mac")
                      if k in entry}
            if fields:
                info[cid] = fields
        return info

    def set_remote_node_id(self, new_id, target=None, by_mac=None,
                           expect_mac=None, qid=1, isBlocking=True, timeout=4):
        """
        Reassign a remote node's CAN id over the bus (SDO write to OD 0x250A).

        Identify the node EITHER by its current id (``target``, optionally with
        ``expect_mac`` for a safety check) OR purely by its MAC (``by_mac``), in
        which case the firmware probes the bus to find which id currently has
        that MAC — use this when you don't know the current id. ``new_id`` is
        always the desired new id.

        The slave persists the new id to NVS and performs a CANopen
        communication reset, so it reappears at ``new_id`` after ~0.3 s.

        :param new_id: desired CAN id (1..127)
        :param target: current CAN id (1..127) of the node to reconfigure
        :param by_mac: target MAC "AA:BB:CC:DD:EE:FF" — firmware finds the node
            (use instead of ``target`` when the current id is unknown)
        :param expect_mac: when using ``target``, verify the node's MAC matches
            before committing (id only ever binds to the intended device)
        :param qid: Query ID (kept for API symmetry)
        :param isBlocking: wait for the firmware acknowledgement
        :param timeout: command timeout in seconds
        :return: firmware response, e.g. ``{"status":"ok","target":60,"newId":70}``
            or ``{"status":"error","error":"MAC not found on bus","mac":"..."}``
        """
        if by_mac is None and target is None:
            raise ValueError("set_remote_node_id requires either target or by_mac")
        path = "/can_act"
        payload = {
            "task": path,
            "setRemoteNodeId": int(new_id),
            "qid": qid,
        }
        if by_mac is not None:
            payload["byMac"] = str(by_mac)
        else:
            payload["target"] = int(target)
            if expect_mac is not None:
                payload["expectMac"] = str(expect_mac)
        nResponses = 1 if isBlocking else 0
        return self._parent.post_json(
            path,
            payload,
            getReturn=isBlocking,
            timeout=timeout if isBlocking else 0,
            nResponses=nResponses,
        )

    def assign_node_id_by_mac(self, mac, new_id, qid=1, timeout=5):
        """
        Assign a CAN id to the node with the given MAC address.

        The firmware probes the bus to locate whichever node currently
        advertises ``mac`` and reassigns it to ``new_id`` in a single round-trip
        (no need to know the node's current id). This is the canonical
        MAC-keyed provisioning call.

        :param mac: target MAC as "AA:BB:CC:DD:EE:FF" (case-insensitive)
        :param new_id: desired CAN id (1..127)
        :param qid: Query ID
        :param timeout: request timeout in seconds
        :return: firmware response, e.g. ``{"status":"ok","target":60,"newId":70,
            "mac":"..."}`` or ``{"status":"error","error":"MAC not found on bus"}``
        """
        return self.set_remote_node_id(new_id, by_mac=mac, qid=qid, timeout=timeout)