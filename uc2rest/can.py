class CAN(object):
    def __init__(self, parent):
        """
        CANController handles sending commands to a remote CAN device via the parent post_json interface.

        :param parent: Parent object with post_json(path, payload, getReturn, timeout, nResponses)
        """
        self._parent = parent

    def reboot_remote(self, qid=1, can_address=0, isBlocking=False, timeout=2):
        """
        Send a reboot signal to the remote CAN device.

        :param qid: Query ID for the CAN command (default: 1)
        :param isBlocking: If True, wait for response
        :param timeout: Timeout for the command in seconds
        :param can_address: Address of the CAN device to reboot (0 is master)
        :return: Response from the device
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