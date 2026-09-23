"""stagescan payload + completion callback, without hardware.

Run: python -m unittest tests.test_stage_scan  (from the repo root)
"""
import unittest
from unittest import mock

from uc2rest.motor import Motor


class _Parent:
    """Just enough of UC2Client for Motor: a serial with register_callback and post_json."""

    def __init__(self):
        self.serial = mock.Mock()
        self.logger = mock.Mock()
        self.sent = []

    def post_json(self, path, payload, **kwargs):
        self.sent.append((path, payload))
        return {"qid": 1, "return": 1}


class StageScanPayloadTest(unittest.TestCase):
    def setUp(self):
        self.parent = _Parent()
        self.motor = Motor(parent=self.parent)
        self.motor.stepSizeX, self.motor.stepSizeY, self.motor.stepSizeZ = 2.0, 4.0, 0.5

    def _scan(self):
        self.assertEqual(len(self.parent.sent), 1)
        path, payload = self.parent.sent[0]
        self.assertEqual(path, "/motor_act")
        self.assertEqual(payload["task"], "/motor_act")
        return payload["stagescan"]

    def test_positions_are_converted_to_steps(self):
        self.motor.start_stage_scanning(xstart=10, xstep=20, nx=3, ystart=40, ystep=8, ny=2,
                                        zstart=1, zstep=2, nz=4)
        s = self._scan()
        self.assertEqual((s["xStart"], s["xStep"], s["nX"]), (5, 10, 3))
        self.assertEqual((s["yStart"], s["yStep"], s["nY"]), (10, 2, 2))
        self.assertEqual((s["zStart"], s["zStep"], s["nZ"]), (2, 4, 4))

    def test_firmware_keys_and_defaults(self):
        self.motor.start_stage_scanning(tsettle=90, tExposure=50)
        s = self._scan()
        self.assertEqual((s["tPre"], s["tPost"]), (90, 50))
        self.assertEqual(s["speed"], 20000)
        self.assertEqual(s["acceleration"], self.motor.DEFAULT_ACCELERATION)
        self.assertNotIn("accel", s)          # the firmware reads "acceleration"
        self.assertNotIn("tTrig", s)          # firmware default unless requested
        self.assertEqual((s["zicZac"], s["nonstop"]), (1, 0))

    def test_illumination_keeps_its_channel_index(self):
        self.motor.start_stage_scanning(illumination=[0, 50], led=None)
        s = self._scan()
        self.assertEqual(s["illumination"], [0, 50, 0, 0, 0])   # padded at the end, never shifted
        self.assertEqual(s["led"], 0)
        self.parent.sent.clear()
        self.motor.start_stage_scanning(illumination=(1, 2, 3, 4, 5, 6), led=255, tTrig=2, nonstop=True, zicZac=False)
        s = self._scan()
        self.assertEqual(s["illumination"], [1, 2, 3, 4, 5])
        self.assertEqual((s["led"], s["tTrig"], s["nonstop"], s["zicZac"]), (255, 2, 1, 0))

    def test_stop_payload(self):
        self.motor.stop_stage_scanning()
        self.assertEqual(self._scan(), {"stopped": 1})

    def test_completion_callback_sets_flag_and_notifies(self):
        got = []
        self.motor.register_stagescan_callback(got.append)
        self.motor.start_stage_scanning()
        self.assertFalse(self.motor._stagescan_complete)
        self.motor._callback_stagescan_complete({"stagescan": 1, "qid": 7, "success": 1})
        self.assertTrue(self.motor._stagescan_complete)
        self.assertEqual(got, [{"stagescan": 1, "qid": 7, "success": 1}])
        self.motor.unregister_stagescan_callback(got.append)
        self.motor._callback_stagescan_complete({"stagescan": 1, "qid": 8, "success": 1})
        self.assertEqual(len(got), 1)


if __name__ == "__main__":
    unittest.main()


class CameraTriggerNotificationTest(unittest.TestCase):
    """{"cam":1} (old firmware) and {"cam":1,"frame":n} (new) both reach the callback."""

    def setUp(self):
        from uc2rest.camera_trigger import CameraTrigger
        self.parent = _Parent()
        self.trigger = CameraTrigger(parent=self.parent)
        self.got = []
        self.trigger.register_callback(0, self.got.append)

    def test_plain_trigger_counts_from_one(self):
        self.trigger._callback_camera_trigger({"cam": 1})
        self.trigger._callback_camera_trigger({"cam": 1})
        self.assertEqual([g["frame_id"] for g in self.got], [1, 2])
        self.assertNotIn("frame", self.got[0])

    def test_numbered_trigger_passes_firmware_frame_through(self):
        self.trigger._callback_camera_trigger({"cam": 1, "frame": 0})
        self.trigger._callback_camera_trigger({"cam": 1, "frame": 5})
        self.assertEqual([g["frame"] for g in self.got], [0, 5])
        self.assertEqual([g["frame_id"] for g in self.got], [1, 2])
