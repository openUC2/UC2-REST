import time
import json

class Gripper(object):
    """
    Example Python class for controlling a gripper servo via JSON commands
    to the MCU, similar in style to the Objective class provided.
    """

    def __init__(self, parent):
        """
        parent: an object that handles the low-level post_json(path, payload, getReturn, timeout, nResponses)
                similar to the 'Home' or 'Objective' parent in the template.
        """
        self._parent = parent
        self.timeout = 10  # default timeout for blocking commands
        # Default angles
        self.close_angle = 0
        self.open_angle  = 180

    def close(self, isBlocking=False):
        """
        Closes the gripper to close_angle (0° by default).
        Sends {"task":"/gripper_act","action":"close"} over Serial/JSON.
        """
        path = "/gripper_act"
        payload = {
            "task": path,
            "action": "close"
        }
        # Possibly wait for the MCU response
        nResponses = 1 if isBlocking else 0
        r = self._parent.post_json(
            path,
            payload,
            getReturn=isBlocking,
            timeout=self.timeout if isBlocking else 0,
            nResponses=nResponses
        )
        return r

    def open(self, isBlocking=False):
        """
        Opens the gripper to open_angle (180° by default).
        Sends {"task":"/gripper_act","action":"open"}.
        """
        path = "/gripper_act"
        payload = {
            "task": path,
            "action": "open"
        }
        nResponses = 1 if isBlocking else 0
        r = self._parent.post_json(
            path,
            payload,
            getReturn=isBlocking,
            timeout=self.timeout if isBlocking else 0,
            nResponses=nResponses
        )
        return r

    def setAngle(self, angle, isBlocking=False):
        """
        Moves the gripper servo to an arbitrary angle (0–180).
        Sends {"task":"/gripper_act","action":"degree","value":<angle>}.
        """
        path = "/gripper_act"
        payload = {
            "task": path,
            "action": "degree",
            "value": angle
        }
        nResponses = 1 if isBlocking else 0
        r = self._parent.post_json(
            path,
            payload,
            getReturn=isBlocking,
            timeout=self.timeout if isBlocking else 0,
            nResponses=nResponses
        )
        return r

    def getstatus(self):
        """
        Query the current gripper status.
        Expects MCU to return something like:
          {
            "gripper":{
              "pos": 90,
              "isOpen": 0 or 1,
              "state": 1,
              "isRunning": 0
            }
          }
        """
        path = "/gripper_get"
        payload = {"task": path}
        r = self._parent.post_json(path, payload, timeout=1)

        try:
            # The last item in r is assumed to contain the "gripper" field
            status = r[-1]["gripper"]
        except:
            # Fallback if parsing fails
            status = {"pos": 0, "isOpen": None, "state": 0, "isRunning": 0}

        return status
