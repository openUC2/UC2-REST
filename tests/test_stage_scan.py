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


class StrobeSweepTest(unittest.TestCase):
    """Strobed sweep payloads, unit conversion and event parsing."""

    def setUp(self):
        self.parent = _Parent()
        self.motor = Motor(parent=self.parent)
        self.motor.stepSizeX = 0.5
        self.motor.direction[1] = -1

    def _sweep(self):
        path, payload = self.parent.sent[-1]
        self.assertEqual(path, "/motor_act")
        return payload["strobesweep"]

    def test_registers_pattern_callbacks(self):
        patterns = [c.kwargs.get("pattern") for c in self.parent.serial.register_callback.call_args_list]
        self.assertIn("strobesweep", patterns)
        self.assertIn("modules", patterns)

    def test_start_payload_converts_target_to_steps(self):
        self.motor.start_strobe_sweep(axis="X", target=1000.0, speed=4000, period_us=20000,
                                      laser=4, delay_us=1200, width_us=20, report=8)
        s = self._sweep()
        self.assertEqual(s["axis"], 1)
        self.assertEqual(s["target"], -2000)          # 1000 µm / 0.5 µm/step, direction -1
        self.assertEqual((s["speed"], s["periodUs"], s["trigUs"]), (4000, 20000, 100))
        self.assertEqual((s["laser"], s["delayUs"], s["widthUs"]), (4, 1200, 20))
        self.assertEqual((s["latch"], s["report"], s["maxFrames"]), (1, 8, 0))
        self.assertNotIn("acceleration", s)

    def test_strobe_keys_only_when_both_given(self):
        self.motor.start_strobe_sweep(axis="X", target=0, laser=4, delay_us=1200)
        s = self._sweep()
        self.assertNotIn("delayUs", s)
        self.assertNotIn("widthUs", s)

    def test_relative_target_and_calibration_mode(self):
        self.motor.currentPosition[1] = 250.0
        self.motor.start_strobe_sweep(axis=1, target=-50.0, is_absolute=False, max_frames=12, latch=False)
        s = self._sweep()
        self.assertEqual(s["target"], -400)            # (250 - 50) / 0.5 * -1
        self.assertEqual((s["maxFrames"], s["latch"], s["laser"]), (12, 0, -1))

    def test_stop_payload(self):
        self.motor.stop_strobe_sweep()
        self.assertEqual(self._sweep(), {"stopped": 1})

    def test_report_and_done_events(self):
        events = []
        self.motor.register_strobesweep_callback(events.append)
        self.motor.start_strobe_sweep(axis="X", target=10)
        self.assertTrue(self.motor.is_strobe_sweep_running())
        self.motor._callback_strobesweep({"strobesweep": {"n": [1, 2, 4], "x": [100, 102, 106]}})
        self.motor._callback_strobesweep({"strobesweep": True, "frames": 4, "camera": 4, "flashes": 4,
                                          "positions": 3, "latch": 1, "strobe": 1, "aborted": 0,
                                          "success": 1, "qid": 5})
        self.assertEqual(events[0], {"type": "report", "n": [1, 2, 4], "x": [-50.0, -51.0, -53.0],
                                     "x_steps": [100, 102, 106]})
        self.assertEqual(events[1]["type"], "done")
        self.assertEqual((events[1]["frames"], events[1]["success"]), (4, 1))
        self.assertFalse(self.motor.is_strobe_sweep_running())
        self.motor.unregister_strobesweep_callback(events.append)
        self.motor._callback_strobesweep({"strobesweep": {"n": [5], "x": [1]}})
        self.assertEqual(len(events), 2)

    def test_capability_probe(self):
        # new firmware answers /modules_get with the flag; the answer arrives by pattern
        def answer(path, payload, **kwargs):
            self.parent.sent.append((path, payload))
            self.motor._callback_modules({"modules": {"motor": 1, "strobesweep": 1}})
        self.parent.post_json = answer
        self.assertTrue(self.motor.has_strobe_sweep(timeout=0.2))
        self.assertEqual(self.parent.sent[-1][1]["task"], "/modules_get")

    def test_capability_probe_old_firmware(self):
        def old(path, payload, **kwargs):
            self.motor._callback_modules({"modules": {"motor": 1}})
        self.parent.post_json = old
        self.assertFalse(self.motor.has_strobe_sweep(timeout=0.2))
        self.parent.post_json = lambda *a, **k: None   # no answer at all
        self.assertFalse(self.motor.has_strobe_sweep(timeout=0.05))


class LaserStrobeTest(unittest.TestCase):
    def setUp(self):
        from uc2rest.laser import Laser
        self.parent = _Parent()
        self.laser = Laser(parent=self.parent)

    def test_payload_and_answer(self):
        self.parent.post_json = lambda path, payload, **kw: (
            self.parent.sent.append((path, payload)) or
            [{"strobe": {"supported": 1, "enabled": 1, "delayUs": 1200, "widthUs": 20, "count": 0},
              "return": 1, "qid": 3}])
        r = self.laser.set_strobe(4, enable=True, delay_us=1200, width_us=20)
        path, payload = self.parent.sent[-1]
        self.assertEqual(path, "/laser_act")
        self.assertEqual(payload["LASERid"], 4)
        self.assertEqual(payload["strobe"], {"enable": 1, "delayUs": 1200, "widthUs": 20})
        self.assertEqual(r["strobe"]["enabled"], 1)

    def test_old_firmware_returns_none(self):
        self.parent.post_json = lambda *a, **k: "communication interrupted by timeout or reset: 3"
        self.assertIsNone(self.laser.set_strobe(4, enable=False))
