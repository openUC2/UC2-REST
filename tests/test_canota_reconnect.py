"""How the CAN streaming OTA worker hands the port back to the parent link.

Run: python -m unittest tests.test_canota_reconnect  (from the repo root)

The parent link must come back at its own baudrate, and without the DTR/RTS
pulse (a hard reset of the master) when the master itself ended the session.
It keeps the reset when the host gave up, because the master may then still
be in binary receive mode.
"""
import json
import os
import tempfile
import unittest
from unittest import mock

import uc2rest.canota as canota
from uc2rest.canota import CANOTA


class FakeStreamPort:
    """The raw pyserial port the worker opens for streaming. `reply` maps the
    /ota_start preamble to the master's answer (None = no answer)."""

    def __init__(self, reply, ack=True, final=None):
        self.reply, self.ack, self.final = reply, ack, final
        self.rx = bytearray()
        self.received = 0
        self.pending = b""  # final status, sent once the last ACK was read

    def _emit(self, obj):
        self.rx += (json.dumps(obj) + "\n").encode()

    def write(self, data):
        if b"/ota_start" in data:
            if self.reply is not None:
                self._emit(self.reply)
            return
        self.received += len(data)
        if self.ack:
            self._emit({"ota_rx": self.received})
        if self.final and self.received >= self.reply.get("size", 0):
            self.pending = (json.dumps(self.final) + "\n").encode()

    @property
    def in_waiting(self):
        if not self.rx and self.pending:
            self.rx, self.pending = bytearray(self.pending), b""
        return len(self.rx)

    def read(self, n=1):
        out, self.rx = bytes(self.rx[:n]), self.rx[n:]
        return out

    def flush(self): pass
    def reset_input_buffer(self): pass
    def isOpen(self): return True
    def close(self): pass


class _Parent:
    def __init__(self, baudrate):
        self.serial = mock.Mock(serialport="/dev/ttyTEST", baudrate=baudrate)
        self.serial.serialdevice.isOpen.return_value = True


FIRMWARE = b"\xe9" + bytes(4999)  # two 4 KB chunks


class ReconnectTest(unittest.TestCase):
    def setUp(self):
        fd, self.firmware = tempfile.mkstemp(suffix=".bin")
        os.write(fd, FIRMWARE)
        os.close(fd)
        self.addCleanup(os.remove, self.firmware)

    def run_upload(self, port, parent_baud=115200):
        parent = _Parent(parent_baud)
        opened = []

        def open_port(p, baud, timeout=None):
            opened.append(baud)
            return port

        with mock.patch.object(canota.serial, "Serial", side_effect=open_port), \
             mock.patch.object(canota.time, "sleep"), \
             mock.patch.object(canota, "READY_TIMEOUT_S", 0.2), \
             mock.patch.object(canota, "ACK_TIMEOUT_S", 0.2), \
             mock.patch.object(canota, "FLASH_TIMEOUT_S", 0.2):
            ok = CANOTA(parent).start_can_streaming_ota_blocking(
                can_id=11, firmware_path=self.firmware)
        return ok, opened, parent.serial.openDevice

    def test_master_refusal_restores_without_reset_at_link_baud(self):
        ok, opened, reopen = self.run_upload(
            FakeStreamPort({"ota_status": "error", "error": "slave_unreachable"}))
        self.assertFalse(ok)
        self.assertEqual(opened, [115200])          # streamed at the link's baud
        reopen.assert_called_once_with("/dev/ttyTEST", 115200, reset=False)

    def test_success_restores_without_reset(self):
        size = len(FIRMWARE)
        port = FakeStreamPort({"ota_status": "ready", "size": size},
                              final={"ota_status": "success"})
        ok, _, reopen = self.run_upload(port, parent_baud=921600)
        self.assertTrue(ok)
        reopen.assert_called_once_with("/dev/ttyTEST", 921600, reset=False)

    def test_silent_master_keeps_the_reset(self):
        ok, _, reopen = self.run_upload(FakeStreamPort(reply=None))
        self.assertFalse(ok)
        reopen.assert_called_once_with("/dev/ttyTEST", 115200, reset=True)

    def test_missing_acks_keep_the_reset(self):
        size = len(FIRMWARE)
        ok, _, reopen = self.run_upload(
            FakeStreamPort({"ota_status": "ready", "size": size}, ack=False))
        self.assertFalse(ok)
        reopen.assert_called_once_with("/dev/ttyTEST", 115200, reset=True)


if __name__ == "__main__":
    unittest.main()
