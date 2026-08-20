"""
Asyncio facade for uc2rest.

Wraps the blocking, thread-based ``UC2Client`` so it can be driven from
asyncio applications (e.g. the newswitch backend) without blocking the event
loop. Every blocking call runs via ``asyncio.to_thread``; firmware events
arriving on the serial read thread (pattern-keyed callbacks such as
``steppers``, ``home``, ``emergency``, ``message``) are bridged onto the event
loop and exposed as a typed async event stream.

This module is purely additive and opt-in:

* the synchronous ``UC2Client`` API is unchanged (ImSwitch keeps working),
* it is NOT imported by ``uc2rest/__init__.py`` so the package still imports
  on older Pythons; import it explicitly::

      from uc2rest.aio import AsyncUC2Client

Requires Python >= 3.9 (``asyncio.to_thread``).
"""

from __future__ import annotations

import sys

if sys.version_info < (3, 9):  # pragma: no cover
    raise ImportError("uc2rest.aio requires Python >= 3.9 (asyncio.to_thread)")

import asyncio
import logging
from dataclasses import dataclass, field
from typing import Any, AsyncIterator, Callable, Optional, Union

from .UC2Client import UC2Client

_log = logging.getLogger(__name__)

_DEFAULT_QUEUE_SIZE = 1024

# Firmware JSON keys we subscribe to on the serial read thread by default.
DEFAULT_EVENT_PATTERNS = (
    "steppers",   # live motor positions
    "home",       # homing state
    "emergency",  # e-stop
    "message",    # key/value events (e.g. hardware buttons)
    "gpio",       # collision detector
    "ptz",        # PTZ keyboard / joystick bridge
    "laser",      # laser status
)


# ---------------------------------------------------------------------------
# Typed events
# ---------------------------------------------------------------------------

@dataclass(frozen=True)
class SteppersEvent:
    """Live stepper positions; ``steppers`` is the raw firmware list
    (``[{"stepperid": 1, "position": ...}, ...]``)."""

    steppers: list = field(default_factory=list)


@dataclass(frozen=True)
class HomeEvent:
    """Homing state update; ``data`` is the raw ``home`` payload."""

    data: dict = field(default_factory=dict)


@dataclass(frozen=True)
class EmergencyEvent:
    """Emergency-stop notification from the firmware."""

    data: dict = field(default_factory=dict)


@dataclass(frozen=True)
class MessageEvent:
    """Key/value firmware event (hardware button presses etc.)."""

    key: Any = None
    data: Any = None


@dataclass(frozen=True)
class RawEvent:
    """Any subscribed pattern without a dedicated event type; carries the
    matched pattern and the full JSON frame."""

    pattern: str = ""
    frame: dict = field(default_factory=dict)


SerialEvent = Union[SteppersEvent, HomeEvent, EmergencyEvent, MessageEvent, RawEvent]

# Firmware axis ids: A=0, X=1, Y=2, Z=3 (matches uc2rest.motor).
AXIS_TO_ID = {"A": 0, "X": 1, "Y": 2, "Z": 3}


def _frame_to_event(pattern: str, frame: dict) -> SerialEvent:
    """Convert a raw firmware JSON frame into a typed event."""
    if pattern == "steppers":
        steppers = frame.get("steppers")
        if isinstance(steppers, dict):
            steppers = [steppers]
        return SteppersEvent(steppers=steppers or [])
    if pattern == "home":
        return HomeEvent(data=frame.get("home") or frame)
    if pattern == "emergency":
        return EmergencyEvent(data=frame.get("emergency") or frame)
    if pattern == "message":
        message = frame.get("message") or {}
        return MessageEvent(key=message.get("key"), data=message.get("data"))
    return RawEvent(pattern=pattern, frame=frame)


# ---------------------------------------------------------------------------
# Async client
# ---------------------------------------------------------------------------

class AsyncUC2Client:
    """Asyncio wrapper around the blocking ``UC2Client`` (serial JSON protocol).

    Construct with ``await AsyncUC2Client.create(serialport=..., ...)`` (same
    kwargs as ``UC2Client``). The wrapped sync client is available as ``.sync``
    for the full ~35-submodule API; ``await client.call(fn, ...)`` runs any
    blocking callable off-loop.

    Note on threading: ``UC2Client`` serializes port access internally
    (write lock + single read thread), so concurrent async calls are safe;
    they are simply queued at the serial port.
    """

    def __init__(
        self,
        client: UC2Client,
        loop: Optional[asyncio.AbstractEventLoop] = None,
        event_patterns: tuple = DEFAULT_EVENT_PATTERNS,
    ):
        if not hasattr(client, "serial"):
            raise RuntimeError(
                "UC2Client has no serial connection; pass serialport= "
                "(use serialport='auto' for port auto-discovery)."
            )
        self._client = client
        self._loop = loop or asyncio.get_running_loop()
        self._queues: list[asyncio.Queue] = []
        self._closed = False

        for pattern in event_patterns:
            self._client.serial.register_callback(
                self._make_pattern_callback(pattern), pattern=pattern
            )

    @classmethod
    async def create(cls, event_patterns: tuple = DEFAULT_EVENT_PATTERNS,
                     **kwargs: Any) -> "AsyncUC2Client":
        """Open the serial connection off-loop and return a ready async client.

        Accepts the same keyword arguments as ``UC2Client`` (serialport,
        baudrate, identity, device_id, requireMaster, ...). A missing/None
        ``serialport`` triggers port auto-discovery (unlike the sync client,
        which requires an explicit port to build its serial link at all).
        """
        # UC2Client only constructs its Serial when serialport is not None;
        # Serial itself falls back to findCorrectSerialDevice() for unknown
        # ports, so map None -> "auto" to get auto-discovery.
        if kwargs.get("serialport") is None:
            kwargs["serialport"] = "auto"
        loop = asyncio.get_running_loop()
        client = await asyncio.to_thread(lambda: UC2Client(**kwargs))
        return cls(client, loop=loop, event_patterns=event_patterns)

    # -- plumbing -----------------------------------------------------------

    @property
    def sync(self) -> UC2Client:
        """The wrapped synchronous client (full uc2rest API surface)."""
        return self._client

    @property
    def is_connected(self) -> bool:
        """Whether the serial link is currently alive."""
        return bool(getattr(self._client, "is_connected", False))

    async def call(self, fn: Callable[..., Any], /, *args: Any, **kwargs: Any) -> Any:
        """Run any blocking callable (e.g. ``client.sync.motor.get_position``) off-loop."""
        return await asyncio.to_thread(fn, *args, **kwargs)

    async def aclose(self) -> None:
        """Close the serial connection and stop the read thread."""
        self._closed = True
        await asyncio.to_thread(self._client.close)

    async def __aenter__(self) -> "AsyncUC2Client":
        return self

    async def __aexit__(self, *args: Any) -> None:
        await self.aclose()

    # -- event bridge (serial read thread -> event loop) ----------------------

    def _make_pattern_callback(self, pattern: str) -> Callable[[dict], None]:
        def _callback(frame: dict) -> None:
            if self._closed:
                return
            try:
                event = _frame_to_event(pattern, frame)
            except Exception as exc:  # defensive: firmware frames vary
                _log.debug("Could not parse %r frame: %s", pattern, exc)
                event = RawEvent(pattern=pattern, frame=frame)
            try:
                self._loop.call_soon_threadsafe(self._publish_on_loop, event)
            except RuntimeError:
                pass  # loop already closed during shutdown

        return _callback

    def _publish_on_loop(self, event: SerialEvent) -> None:
        for queue in list(self._queues):
            if queue.full():
                try:
                    queue.get_nowait()  # drop oldest
                except asyncio.QueueEmpty:
                    pass
            queue.put_nowait(event)

    async def events(self, queue_size: int = _DEFAULT_QUEUE_SIZE) -> AsyncIterator[SerialEvent]:
        """Yield typed firmware events as they arrive.

        Multiple consumers may iterate concurrently; each gets its own queue.
        The oldest events are dropped if a consumer falls behind.
        """
        queue: asyncio.Queue = asyncio.Queue(maxsize=queue_size)
        self._queues.append(queue)
        try:
            while True:
                yield await queue.get()
        finally:
            self._queues.remove(queue)

    # -- motor ----------------------------------------------------------------

    async def move_axis(self, axis: str = "X", steps: int = 0, speed: int = 20000,
                        acceleration: Optional[int] = None, is_absolute: bool = False,
                        is_blocking: bool = True, timeout: float = 60.0) -> Any:
        """Move a named axis ("X"/"Y"/"Z"/"A") by/to ``steps``.

        With ``is_blocking=True`` the firmware-side wait runs in a worker
        thread, so awaiting this returns when the motion is done. On
        cancellation the axis is stopped before CancelledError propagates.
        """
        try:
            return await asyncio.to_thread(
                self._client.motor.move_axis_by_name,
                axis=axis, steps=steps, speed=speed, acceleration=acceleration,
                is_absolute=is_absolute, is_blocking=is_blocking, timeout=timeout,
            )
        except asyncio.CancelledError:
            await asyncio.shield(self.stop(axis=axis))
            raise

    async def stop(self, axis: Optional[str] = None) -> None:
        """Stop one axis (or all axes when ``axis`` is None)."""
        await asyncio.to_thread(self._client.motor.stop, axis)

    async def get_positions(self, timeout: float = 1.0) -> Any:
        """Read all motor positions (steps), as reported by the firmware."""
        return await asyncio.to_thread(self._client.motor.get_position, None, timeout)

    async def home_axis(self, axis: str = "X", speed: Optional[int] = None,
                        direction: Optional[int] = None, is_blocking: bool = True,
                        timeout: Optional[float] = None) -> Any:
        """Home a named axis; blocking variant waits for completion off-loop."""
        try:
            return await asyncio.to_thread(
                self._client.home.home,
                axis=axis, timeout=timeout, speed=speed, direction=direction,
                isBlocking=is_blocking,
            )
        except asyncio.CancelledError:
            await asyncio.shield(
                asyncio.to_thread(self._client.home.stop_home, axis)
            )
            raise

    # -- illumination -----------------------------------------------------------

    async def set_laser(self, channel: int = 1, value: int = 0) -> Any:
        """Set a laser/LED PWM channel value."""
        return await asyncio.to_thread(self._client.laser.set_laser, channel, value)

    async def led_fill(self, r: int = 255, g: int = 255, b: int = 255) -> Any:
        """Fill the full LED matrix with one colour."""
        return await asyncio.to_thread(
            self._client.led.send_LEDMatrix_full, (r, g, b)
        )

    async def led_off(self) -> Any:
        """Turn the whole LED matrix off."""
        return await asyncio.to_thread(self._client.led.send_LEDMatrix_full, (0, 0, 0))

    # -- objective changer --------------------------------------------------------

    async def objective_move(self, slot: int = 1, is_blocking: bool = True) -> Any:
        """Move the objective slider to a slot (1-based)."""
        return await asyncio.to_thread(
            lambda: self._client.objective.move(slot=slot, isBlocking=is_blocking)
        )

    async def objective_home(self, is_blocking: bool = True) -> Any:
        """Home the objective slider."""
        return await asyncio.to_thread(
            lambda: self._client.objective.home(isBlocking=is_blocking)
        )

    async def objective_calibrate(self, is_blocking: bool = True) -> Any:
        """Calibrate the objective slider end positions."""
        return await asyncio.to_thread(
            lambda: self._client.objective.calibrate(isBlocking=is_blocking)
        )

    async def objective_status(self) -> Any:
        """Read the objective slider status."""
        return await asyncio.to_thread(self._client.objective.getstatus)

    # -- CAN fleet ----------------------------------------------------------------

    async def can_scan(self) -> Any:
        """Ask the master firmware for the CAN devices it can reach."""
        return await asyncio.to_thread(self._client.can.get_available_devices)

    # -- galvo -------------------------------------------------------------------

    async def galvo_goto(self, x: int, y: int) -> Any:
        """Move the galvo to an absolute XY position (DAC counts)."""
        return await asyncio.to_thread(self._client.galvo.set_position, x, y)

    async def galvo_scan(self, **kwargs: Any) -> Any:
        """Configure and start a galvo scan (see ``Galvo.set_galvo_scan`` kwargs)."""
        return await asyncio.to_thread(lambda: self._client.galvo.set_galvo_scan(**kwargs))

    async def galvo_stop(self) -> Any:
        """Stop any active galvo scan."""
        return await asyncio.to_thread(self._client.galvo.stop_galvo_scan)

    async def galvo_status(self) -> Any:
        """Read the galvo status."""
        return await asyncio.to_thread(self._client.galvo.get_galvo_status)

    # -- system --------------------------------------------------------------------

    async def get_firmware_info(self) -> Any:
        """Read firmware identity ({name, version, date, author, pindef, isMaster})."""
        return await asyncio.to_thread(self._client.state.get_firmware_info)

    async def ping(self, timeout: float = 0.5) -> bool:
        """Check link liveness."""
        return await asyncio.to_thread(self._client.serial.ping, timeout)
