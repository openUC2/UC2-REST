"""/motor_get answers vs. position pushes that share their qid (no hardware).

The firmware tags a motor's position push ({"steppers":[{.., "isDone": 1}]}) with the qid of
that motor's last move; uc2rest's qids wrap after 254 commands, so such a frame can carry the
qid of a later /motor_get. Taken as the answer, every other axis read 0 (ImSwitch FRAME
calibration, 2026-11-12: "X reads 728.2 after a move to 19394.5" = device X 0).

Run: python -m unittest tests.test_motor_get_reply  (from the repo root)
"""
import threading
import time
import unittest
from unittest import mock

import numpy as np

from uc2rest.motor import Motor
from uc2rest.mserial import Serial

PUSH = {"steppers": [{"stepperid": 0, "position": -29000, "isDone": 1}], "qid": 7}
REPLY = {"motor": {"steppers": [{"stepperid": i, "position": p} for i, p in enumerate((-29000, 19000, 60000, 5900))]},
         "qid": 7}


class _Parent:
    def __init__(self, frames):
        self.serial = mock.Mock()
        self.logger = mock.Mock()
        self.frames = frames

    def post_json(self, path, payload, accept=None, **kwargs):
        # like Serial.sendMessage: with accept, the matching frame first
        if accept is not None:
            hit = next((f for f in self.frames if accept(f)), None)
            return "communication interrupted by timeout" if hit is None else [hit] + [f for f in self.frames if f is not hit]
        return list(self.frames)


class MotorGetReplyTest(unittest.TestCase):
    def motor(self, frames):
        parent = _Parent(frames)
        m = parent.motor = Motor(parent=parent)
        for ax in "AXYZ":
            m.setup_motor(ax, minPos=-1e9, maxPos=1e9, stepSize=1, backlash=0)
        return m

    def test_push_with_our_qid_is_not_the_answer(self):
        p = self.motor([PUSH, REPLY]).get_position()
        self.assertEqual(list(p[:4]), [-29000, 19000, 60000, 5900])

    def test_no_answer_reports_missing(self):
        m = self.motor([PUSH])
        self.assertTrue(np.isnan(m.get_position(missing=np.nan)[:4]).all())
        self.assertEqual(list(m.get_position()[:4]), [0, 0, 0, 0])        # default: as before
        self.assertEqual(m._read_firmware_positions(), {})

    def test_filtered_can_reply_is_an_answer(self):
        can = {"steppers": [{"stepperid": 4, "position": 3}, {"stepperid": 5, "isDone": -1}], "qid": 2}
        self.assertTrue(Motor._is_motor_get_reply(can, ids=[4, 5]))
        self.assertFalse(Motor._is_motor_get_reply(can))                 # a bare request wants {"motor": ...}
        self.assertFalse(Motor._is_motor_get_reply(PUSH, ids=[0, 1]))
        self.assertFalse(Motor._is_motor_get_reply(PUSH))
        self.assertFalse(Motor._is_motor_get_reply("communication interrupted"))


class SendMessageAcceptTest(unittest.TestCase):
    """Serial.sendMessage waits past frames that share the qid but fail accept."""

    def test_waits_for_the_accepted_frame(self):
        s = Serial.__new__(Serial)
        s.lock, s.responses, s.commands = threading.Lock(), {}, {}
        s.identifier_counter, s.running, s.is_connected = 0, True, True
        s.resetLastCommand, s.use_qid_done, s.DEBUG, s.manufacturer = False, False, False, "test"
        s.timeReturnReceived, s._logger, s.cmdWriteCallBackFct = 5, mock.Mock(), None

        def write(dev, payload):
            qid = s.identifier_counter
            def arrive():
                for f in ({**PUSH, "qid": qid}, {**REPLY, "qid": qid}):
                    time.sleep(0.05)
                    with s.lock:
                        s.responses.setdefault(qid, []).append(f)
            threading.Thread(target=arrive, daemon=True).start()
        s._write, s.serialdevice = write, None
        r = s.sendMessage({"task": "/motor_get", "position": True}, nResponses=1, timeout=2,
                          accept=Motor._is_motor_get_reply)
        self.assertIn("motor", r[0])
        self.assertEqual(len(r), 2)


if __name__ == "__main__":
    unittest.main()
