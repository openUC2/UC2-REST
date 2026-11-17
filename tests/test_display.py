"""Status OLED (/display_act, /display_get) payloads and text handling, without hardware.

Run: python -m unittest tests.test_display  (from the repo root)
"""
import unittest
from unittest import mock

from uc2rest.display import Display, _ascii

REPLY = [{"display": {"present": 1, "on": 1, "addr": 60, "name": "openuc2-metal-slope-23501", "status": "",
                      "progress": -1, "fw": "v2026.0.0-beta.4", "flip": 0, "contrast": 143, "splash": 0},
          "qid": 0}]


class _Parent:
    def __init__(self, reply=None):
        self.serial = mock.Mock()
        self.reply = reply
        self.sent = []

    def post_json(self, path, payload, **kwargs):
        self.sent.append((path, payload, kwargs))
        return self.reply

    def get_json(self, path, **kwargs):
        self.sent.append((path, None, kwargs))
        return self.reply


class DisplayTest(unittest.TestCase):
    def test_get_display(self):
        display = Display(_Parent(REPLY))
        self.assertEqual(display.get_display()["name"], "openuc2-metal-slope-23501")
        self.assertTrue(display.is_present())

    def test_get_display_without_module(self):
        # firmware without the module answers something unrelated / nothing at all
        self.assertEqual(Display(_Parent([{"error": "unknown task"}])).get_display(), {})
        self.assertEqual(Display(_Parent(None)).get_display(), {})
        self.assertFalse(Display(_Parent(None)).is_present())

    def test_get_display_reports_missing_panel(self):
        display = Display(_Parent([{"display": {"present": 0, "on": 1}}]))
        self.assertFalse(display.is_present())

    def test_payloads(self):
        parent = _Parent()
        display = Display(parent)
        display.set_name("openuc2-metal-slope-23501")
        display.set_status("Scanning 12/340", progress=3)
        display.set_progress(250)      # clamped
        display.clear_status()
        display.off()
        display.on()
        display.set_flip(True)
        display.set_contrast(999)      # clamped
        display.show_splash()
        payloads = [p for _, p, _ in parent.sent]
        self.assertEqual(payloads[0], {"task": "/display_act", "display": {"name": "openuc2-metal-slope-23501"}})
        self.assertEqual(payloads[1]["display"], {"status": "Scanning 12/340", "progress": 3})
        self.assertEqual(payloads[2]["display"], {"progress": 100})
        self.assertEqual(payloads[3]["display"], {"status": "", "progress": -1})
        self.assertEqual(payloads[4]["display"], {"on": 0})
        self.assertEqual(payloads[5]["display"], {"on": 1})
        self.assertEqual(payloads[6]["display"], {"flip": 1})
        self.assertEqual(payloads[7]["display"], {"contrast": 255})
        self.assertEqual(payloads[8]["display"], {"splash": 1})
        self.assertTrue(all(path == "/display_act" and kw["getReturn"] is False for path, _, kw in parent.sent))

    def test_non_ascii_is_transliterated(self):
        self.assertEqual(_ascii("Focus map Z → 12.30 (5.0 µm)"), "Focus map Z > 12.30 (5.0 um)")
        self.assertEqual(_ascii("Größe"), "Grosse")
        self.assertEqual(_ascii("日本"), "")
        self.assertEqual(_ascii("a\tb\nc"), "a b c")
        self.assertEqual(_ascii(None), "")
        self.assertEqual(len(_ascii("x" * 200)), 63)

    def test_progress_garbage_hides_bar(self):
        parent = _Parent()
        Display(parent).set_progress("n/a")
        self.assertEqual(parent.sent[-1][1]["display"], {"progress": -1})


if __name__ == "__main__":
    unittest.main()
