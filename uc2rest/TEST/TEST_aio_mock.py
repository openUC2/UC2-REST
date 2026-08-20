"""Hardware-free test for uc2rest.aio: MockSerial fallback + typed event bridge.

Run:  python uc2rest/TEST/TEST_aio_mock.py
"""

import asyncio
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))

from uc2rest.aio import (  # noqa: E402
    AsyncUC2Client,
    EmergencyEvent,
    MessageEvent,
    SteppersEvent,
)


def _inject_frame(client, frame: dict) -> None:
    """Push a fake firmware JSON frame through the registered serial callbacks."""
    for entry in client.sync.serial.callBackList:
        if entry["pattern"] in frame:
            entry["callbackfct"](frame)


def test_mock_connect_and_events() -> None:
    """Facade constructs on MockSerial and bridges pattern frames to typed events."""

    async def scenario():
        client = await AsyncUC2Client.create(serialport="auto", skipFirmwareCheck=True)
        assert client.is_connected is False  # mock fallback, no hardware
        assert client.sync.serial.manufacturer == "UC2Mock"

        received = []

        async def collect():
            async for event in client.events():
                received.append(event)
                if len(received) >= 3:
                    return

        collector = asyncio.create_task(collect())
        await asyncio.sleep(0)

        _inject_frame(client, {"steppers": [{"stepperid": 1, "position": 4200}]})
        _inject_frame(client, {"message": {"key": "btnA", "data": 1}})
        _inject_frame(client, {"emergency": {"active": 1}})

        await asyncio.wait_for(collector, timeout=2.0)
        await client.aclose()
        return received

    received = asyncio.run(scenario())

    steppers = [e for e in received if isinstance(e, SteppersEvent)]
    assert len(steppers) == 1 and steppers[0].steppers[0]["position"] == 4200
    messages = [e for e in received if isinstance(e, MessageEvent)]
    assert len(messages) == 1 and messages[0].key == "btnA"
    emergencies = [e for e in received if isinstance(e, EmergencyEvent)]
    assert len(emergencies) == 1


if __name__ == "__main__":
    test_mock_connect_and_events()
    print("PASS TEST_aio_mock")
