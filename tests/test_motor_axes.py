"""Generic motor axes (stepperid 0-9, named axes), without hardware.

Run: python -m unittest tests.test_motor_axes  (from the repo root)
"""
import unittest
from unittest import mock

from uc2rest.home import Home
from uc2rest.motor import Motor


class _Parent:
    """Just enough of UC2Client: a serial with register_callback, post_json and a motor."""

    def __init__(self):
        self.serial = mock.Mock()
        self.logger = mock.Mock()
        self.sent = []
        self.reply = {"qid": 1, "return": 1}

    def post_json(self, path, payload, **kwargs):
        self.sent.append((path, payload))
        return self.reply


class MotorAxesTest(unittest.TestCase):
    def setUp(self):
        self.parent = _Parent()
        self.motor = self.parent.motor = Motor(parent=self.parent)

    def steppers(self):
        return self.parent.sent[-1][1]["motor"]["steppers"]

    def test_named_axis_beyond_xyza(self):
        self.motor.setup_motor("C", minPos=0, maxPos=10, stepSize=0.5, backlash=0)
        self.motor.move_axis_by_name("C", 10, speed=1000)
        self.assertEqual([(s["stepperid"], s["position"]) for s in self.steppers()], [(5, 20)])
        self.motor.move_axis_by_name(7, 3, speed=1000)  # by stepperid
        self.assertEqual(self.steppers()[0]["stepperid"], 7)

    def test_move_axes_mixed_names(self):
        self.motor.move_axes({"X": 100, "B": -50}, speed={"X": 2000, "B": 3000})
        got = {s["stepperid"]: (s["position"], s["speed"]) for s in self.steppers()}
        self.assertEqual(got, {1: (100, 2000), 4: (-50, 3000)})

    def test_relative_zero_axes_are_not_sent(self):
        self.motor.move_xyz((0, 5, 0), speed=1000)
        self.assertEqual([s["stepperid"] for s in self.steppers()], [2])

    def test_remapped_and_custom_names(self):
        self.motor.setMotorAxisOrder([0, 2, 1, 3])  # legacy: X and Y swapped
        self.motor.move_x(10, speed=1000)
        self.assertEqual(self.steppers()[0]["stepperid"], 2)
        self.motor.set_axis_ids({"T": 6})
        self.motor.move_axis_by_name("t", 1, speed=1000)
        self.assertEqual(self.steppers()[0]["stepperid"], 6)
        with self.assertRaises(ValueError):
            self.motor.axis_id("Q")
        with self.assertRaises(ValueError):
            self.motor.axis_id(10)

    def test_feedback_reaches_every_callback(self):
        seen = {}
        self.motor.register_callback("stage1", lambda p: seen.setdefault("a", p[6]))
        self.motor.register_callback("stage2", lambda p: seen.setdefault("b", p[6]))
        self.motor.setup_motor("D", 0, 0, stepSize=2, backlash=0, direction=-1)
        self.motor._callback_motor_status(
            {"steppers": [{"stepperid": 6, "position": 10}, {"stepperid": 42, "position": 1}]})
        self.assertEqual(seen, {"a": -20, "b": -20})

    def test_stop_all_sends_only_known_ids(self):
        self.motor.move_axis_by_name("E", 5, speed=1000)
        self.motor.stop()
        self.assertEqual([s["stepperid"] for s in self.steppers()], [0, 1, 2, 3, 7])

    def test_move_forever_only_given_axes(self):
        self.motor.move_forever({"C": 500})
        self.assertEqual(self.steppers(), [{"stepperid": 5, "isforever": 1, "speed": 500}])

    def test_get_position_filtered(self):
        self.parent.reply = [{"steppers": [{"stepperid": 4, "position": 8}, {"stepperid": 5, "isDone": -1}]}]
        self.motor.setup_motor("B", 0, 0, stepSize=0.25, backlash=0)
        pos = self.motor.get_position(axis=["B", "C"])
        self.assertEqual(self.parent.sent[-1][1]["motor"], {"steppers": [{"stepperid": 4}, {"stepperid": 5}]})
        self.assertEqual((len(pos), pos[4], pos[5]), (10, 2, 0))

    def test_legacy_step_size_attributes(self):
        self.motor.stepSizeX = 2.0
        self.assertEqual((self.motor.stepSize[1], self.motor.stepSizeX), (2.0, 2.0))

    def test_backlash_only_on_reversal(self):
        self.motor.set_backlash("X", 10)
        self.motor.move_x(100, speed=1000)   # first move counts as a reversal
        self.assertEqual(self.steppers()[0]["position"], 110)
        self.motor.move_y(5, speed=1000)     # another axis must not reset X's direction
        self.motor.move_x(100, speed=1000)
        self.assertEqual(self.steppers()[0]["position"], 100)
        self.motor.move_x(-100, speed=1000)
        self.assertEqual(self.steppers()[0]["position"], -110)
        self.motor.move_x(0, speed=1000, is_absolute=True)    # still moving down
        self.assertEqual(self.steppers()[0]["position"], 0)
        self.motor.move_x(50, speed=1000, is_absolute=True)   # up again: overshoot upwards
        self.assertEqual(self.steppers()[0]["position"], 60)

    def test_home_named_axis(self):
        home = Home(self.parent)
        home.home(axis="C", speed=1000)
        self.assertEqual(self.parent.sent[-1][1]["home"]["steppers"][0]["stepperid"], 5)
        home.home_x()
        self.assertEqual(self.parent.sent[-1][1]["home"]["steppers"][0]["stepperid"], 1)


if __name__ == "__main__":
    unittest.main()
