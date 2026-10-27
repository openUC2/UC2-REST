"""/laser_get parameter query (max value, resolution, ...), without hardware.

Run: python -m unittest tests.test_laser_get  (from the repo root)
"""
import unittest
from unittest import mock

from uc2rest.laser import Laser

REPLY = [{"laser": {"LASER1pin": 12, "LASER1val": 300,
                    "lasers": [{"LASERid": 1, "LASERval": 300, "LASERmax": 1024, "LASERRes": 10, "LASERFreq": 5000},
                               {"LASERid": 2, "node": 30, "error": "SDO read failed"}]},
          "qid": 0}]


class _Parent:
    def __init__(self, reply):
        self.serial = mock.Mock()
        self.reply = reply
        self.sent = []

    def post_json(self, path, payload, **kwargs):
        self.sent.append(payload)
        return self.reply


class LaserGetTest(unittest.TestCase):
    def test_query(self):
        laser = Laser(_Parent(REPLY))
        self.assertEqual(laser.get_laser_max(1), 1024)
        self.assertEqual(laser.get_laser(1)["LASERRes"], 10)
        self.assertIsNone(laser.get_laser_max(2))  # CAN slave did not answer
        self.assertEqual(sorted(laser.get_laser()), [1, 2])

    def test_old_firmware(self):
        laser = Laser(_Parent([{"laser": {"LASER1pin": 12, "LASER1val": 0}}]))
        self.assertEqual(laser.get_laser(), {})
        self.assertIsNone(laser.get_laser_max(1))

    def test_callback_reads_get_answer(self):
        laser = Laser(_Parent(REPLY))
        laser._callback_laser_status(REPLY[0])
        self.assertEqual(laser.laserValues[1], 300)
        laser._callback_laser_status({"laser": {"LASERid": 3, "LASERval": 7, "isDone": 1}})
        self.assertEqual(laser.laserValues[3], 7)

    def test_set_pwm_sends_value(self):
        parent = _Parent({"return": 1})
        Laser(parent).set_laser_pwm(1, resolution=12)
        self.assertEqual(parent.sent[-1], {"task": "/laser_act", "LASERid": 1, "LASERval": 0, "LASERRes": 12})


if __name__ == "__main__":
    unittest.main()
