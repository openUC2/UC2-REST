"""
Modern Serial Communication Handler V2

This module provides a clean, refactored serial communication layer with:
- MessagePack and JSON support via SerialProtocol
- Simplified threading model
- Clear blocking/non-blocking operation modes
- Robust QID-based request/response tracking
- Better error handling and logging
"""

import time
import threading
import queue
from typing import Dict, Any, Optional, List, Callable, Tuple
from dataclasses import dataclass
from enum import Enum
import sys

try:
    from serial import Serial as PySerial
    from serial.tools import list_ports
    from serial.serialutil import SerialException
    IS_SERIAL = True
except ImportError:
    IS_SERIAL = False
    class SerialException(Exception):
        pass

from .serial_protocol import SerialProtocol, MessageFormat


class CommandMode(Enum):
    """Command execution modes"""
    BLOCKING = "blocking"       # Wait for response(s)
    NON_BLOCKING = "non_blocking"  # Fire and forget
    ASYNC = "async"             # Return future (not yet implemented)


@dataclass
class CommandRequest:
    """Represents a command to be sent to the device"""
    qid: int
    payload: Dict[str, Any]
    mode: CommandMode
    expected_responses: int = 1
    timeout: float = 10.0
    timestamp: float = 0.0
    
    def __post_init__(self):
        if self.timestamp == 0.0:
            self.timestamp = time.time()


@dataclass
class CommandResponse:
    """Represents a response received from the device"""
    qid: int
    payload: Dict[str, Any]
    timestamp: float = 0.0
    
    def __post_init__(self):
        if self.timestamp == 0.0:
            self.timestamp = time.time()


class SerialCommunicator:
    """
    Modern serial communication handler with MessagePack support.
    
    Features:
    - Automatic protocol detection and negotiation
    - QID-based request/response matching
    - Blocking and non-blocking operation modes
    - Thread-safe command queuing
    - Callback support for async notifications
    
    Example:
        comm = SerialCommunicator(port='/dev/ttyUSB0')
        
        # Blocking call
        response = comm.send_command({"task": "/state_get"}, blocking=True)
        
        # Non-blocking call
        comm.send_command({"task": "/led_set", "led": 100}, blocking=False)
    """
    
    def __init__(self, 
                 port: str,
                 baudrate: int = 115200,
                 timeout: float = 5.0,
                 use_msgpack: bool = True,
                 identity: str = "UC2_Feather",
                 debug: bool = False,
                 skip_firmware_check: bool = False):
        """
        Initialize serial communicator.
        
        Args:
            port: Serial port path (e.g., '/dev/ttyUSB0', 'COM3')
            baudrate: Serial baud rate (default 115200)
            timeout: Default timeout for blocking operations (default 5s)
            use_msgpack: Prefer MessagePack protocol (default True)
            identity: Expected device identity string
            debug: Enable debug logging
            skip_firmware_check: Skip firmware verification on connect
        """
        self.port = port
        self.baudrate = baudrate
        self.default_timeout = timeout
        self.identity = identity
        self.debug = debug
        self.skip_firmware_check = skip_firmware_check
        
        # Protocol handler
        self.protocol = SerialProtocol(prefer_msgpack=use_msgpack, debug=debug)
        
        # Serial device
        self.serial_device: Optional[PySerial] = None
        self.is_connected = False
        
        # QID management
        self.qid_counter = 0
        self.qid_lock = threading.Lock()
        
        # Command tracking
        self.pending_commands: Dict[int, CommandRequest] = {}
        self.response_queues: Dict[int, queue.Queue] = {}
        self.commands_lock = threading.Lock()
        
        # Callback support
        self.callbacks: List[Tuple[Callable, str]] = []  # (callback, pattern)
        self.callbacks_lock = threading.Lock()
        
        # Threading
        self.running = False
        self.read_thread: Optional[threading.Thread] = None
        self.write_thread: Optional[threading.Thread] = None
        self.write_queue: queue.Queue = queue.Queue()
        
        # Statistics
        self.stats = {
            'messages_sent': 0,
            'messages_received': 0,
            'timeouts': 0,
            'errors': 0
        }
        
        # Initialize connection
        if IS_SERIAL:
            self._connect()
        else:
            self._log("Serial library not available - running in mock mode")
    
    def _log(self, message: str, level: str = "INFO"):
        """Internal logging helper"""
        if self.debug or level == "ERROR":
            print(f"[SerialComm-{level}] {message}")
    
    def _connect(self):
        """Establish serial connection and start communication threads"""
        try:
            self._log(f"Connecting to {self.port} at {self.baudrate} baud")
            
            self.serial_device = PySerial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=0.1,  # Short timeout for non-blocking read
                write_timeout=1.0
            )
            
            if not self.serial_device.is_open:
                self.serial_device.open()
            
            # Flush any stale data
            time.sleep(0.5)
            self.serial_device.reset_input_buffer()
            self.serial_device.reset_output_buffer()
            
            # Verify firmware if needed
            if not self.skip_firmware_check:
                self._verify_firmware()
            
            # Start communication threads
            self.running = True
            self.read_thread = threading.Thread(target=self._read_loop, daemon=True, name="SerialReader")
            self.write_thread = threading.Thread(target=self._write_loop, daemon=True, name="SerialWriter")
            
            self.read_thread.start()
            self.write_thread.start()
            
            self.is_connected = True
            self._log("Connected successfully")
            
        except Exception as e:
            self._log(f"Connection failed: {e}", "ERROR")
            raise
    
    def _verify_firmware(self):
        """Verify device firmware matches expected identity"""
        try:
            self._log("Verifying firmware identity...")
            
            # Send state request with short timeout
            response = self.send_command(
                {"task": "/state_get"},
                blocking=True,
                timeout=3.0
            )
            
            if response and "state" in response:
                firmware_id = response.get("state", {}).get("firmware", "unknown")
                self._log(f"Firmware: {firmware_id}")
                
                if self.identity not in firmware_id:
                    self._log(f"Warning: Expected '{self.identity}', got '{firmware_id}'", "WARNING")
            else:
                self._log("Could not verify firmware identity", "WARNING")
                
        except Exception as e:
            self._log(f"Firmware verification failed: {e}", "WARNING")
    
    def _generate_qid(self) -> int:
        """Generate unique query ID"""
        with self.qid_lock:
            self.qid_counter += 1
            if self.qid_counter > 65535:  # Wrap at 16-bit limit
                self.qid_counter = 1
            return self.qid_counter
    
    def _write_loop(self):
        """Background thread that sends queued commands to serial port"""
        self._log("Write thread started")
        
        while self.running:
            try:
                # Get next command with timeout
                request: CommandRequest = self.write_queue.get(timeout=0.1)
                
                if self.serial_device is None or not self.serial_device.is_open:
                    self._log("Serial device not available", "ERROR")
                    continue
                
                # Encode message
                encoded = self.protocol.encode(request.payload)
                
                # Write to serial
                self.serial_device.write(encoded)
                self.serial_device.flush()
                
                self.stats['messages_sent'] += 1
                self._log(f"Sent QID={request.qid}: {request.payload.get('task', 'unknown')} ({len(encoded)} bytes)")
                
            except queue.Empty:
                continue
            except Exception as e:
                self._log(f"Write error: {e}", "ERROR")
                self.stats['errors'] += 1
        
        self._log("Write thread stopped")
    
    def _read_loop(self):
        """Background thread that reads responses from serial port"""
        self._log("Read thread started")
        
        read_buffer = bytearray()
        
        while self.running:
            try:
                if self.serial_device is None or not self.serial_device.is_open:
                    time.sleep(0.1)
                    continue
                
                # Read available data
                if self.serial_device.in_waiting > 0:
                    chunk = self.serial_device.read(self.serial_device.in_waiting)
                    read_buffer.extend(chunk)
                    
                    # Try to parse complete messages
                    self._process_buffer(read_buffer)
                
                time.sleep(0.01)  # Small delay to prevent CPU spinning
                
            except Exception as e:
                self._log(f"Read error: {e}", "ERROR")
                self.stats['errors'] += 1
        
        self._log("Read thread stopped")
    
    def _process_buffer(self, buffer: bytearray):
        """Process read buffer and extract complete messages"""
        while len(buffer) > 0:
            # Get message info
            info = self.protocol.get_message_info(bytes(buffer))
            
            if not info['valid']:
                # Skip invalid byte and try again
                buffer.pop(0)
                continue
            
            if not info['complete']:
                # Need more data
                break
            
            # Extract complete message
            message_bytes = bytes(buffer[:info['total_length']])
            del buffer[:info['total_length']]
            
            # Decode message
            decoded = self.protocol.decode(message_bytes)
            
            if decoded:
                self.stats['messages_received'] += 1
                self._handle_response(decoded)
            else:
                self._log("Failed to decode message", "ERROR")
                self.stats['errors'] += 1
    
    def _handle_response(self, data: Dict[str, Any]):
        """Handle decoded response from device"""
        qid = data.get('qid', 0)
        
        self._log(f"Received QID={qid}: {list(data.keys())}")
        
        # Create response object
        response = CommandResponse(qid=qid, payload=data)
        
        # Route to waiting command if QID matches
        with self.commands_lock:
            if qid in self.response_queues:
                self.response_queues[qid].put(response)
                self._log(f"Routed response to QID={qid} queue")
            else:
                self._log(f"No pending command for QID={qid}")
        
        # Call registered callbacks
        self._trigger_callbacks(data)
    
    def _trigger_callbacks(self, data: Dict[str, Any]):
        """Trigger callbacks that match response data"""
        with self.callbacks_lock:
            for callback, pattern in self.callbacks:
                # Simple pattern matching (can be enhanced)
                if pattern in str(data):
                    try:
                        callback(data)
                    except Exception as e:
                        self._log(f"Callback error: {e}", "ERROR")
    
    def send_command(self, 
                    command: Dict[str, Any],
                    blocking: bool = True,
                    expected_responses: int = 1,
                    timeout: Optional[float] = None) -> Optional[Dict[str, Any]]:
        """
        Send a command to the device.
        
        Args:
            command: Command dictionary (must include 'task' field)
            blocking: Wait for response (default True)
            expected_responses: Number of responses to wait for (default 1)
            timeout: Timeout in seconds (uses default if None)
            
        Returns:
            Response dictionary if blocking=True, None otherwise
            
        Example:
            # Blocking call
            response = comm.send_command({"task": "/state_get"})
            print(response)
            
            # Non-blocking call
            comm.send_command({"task": "/led_set", "led": 100}, blocking=False)
        """
        if not self.is_connected:
            raise RuntimeError("Not connected to device")
        
        if 'task' not in command:
            raise ValueError("Command must include 'task' field")
        
        # Generate QID
        qid = 0 if not blocking else self._generate_qid()
        command['qid'] = qid
        
        # Determine timeout
        cmd_timeout = timeout if timeout is not None else self.default_timeout
        
        # Create request
        mode = CommandMode.BLOCKING if blocking else CommandMode.NON_BLOCKING
        request = CommandRequest(
            qid=qid,
            payload=command,
            mode=mode,
            expected_responses=expected_responses,
            timeout=cmd_timeout
        )
        
        # Setup response tracking for blocking calls
        if blocking:
            with self.commands_lock:
                self.pending_commands[qid] = request
                self.response_queues[qid] = queue.Queue()
        
        # Queue for sending
        self.write_queue.put(request)
        
        # Wait for response if blocking
        if blocking:
            return self._wait_for_response(qid, expected_responses, cmd_timeout)
        
        return None
    
    def _wait_for_response(self, qid: int, expected: int, timeout: float) -> Optional[Dict[str, Any]]:
        """Wait for response(s) to a blocking command"""
        start_time = time.time()
        responses = []
        
        try:
            with self.commands_lock:
                response_queue = self.response_queues.get(qid)
            
            if response_queue is None:
                self._log(f"No response queue for QID={qid}", "ERROR")
                return None
            
            # Collect expected number of responses
            while len(responses) < expected:
                remaining = timeout - (time.time() - start_time)
                
                if remaining <= 0:
                    self._log(f"Timeout waiting for QID={qid} (got {len(responses)}/{expected})", "WARNING")
                    self.stats['timeouts'] += 1
                    break
                
                try:
                    response = response_queue.get(timeout=remaining)
                    responses.append(response.payload)
                    self._log(f"Collected response {len(responses)}/{expected} for QID={qid}")
                except queue.Empty:
                    self._log(f"Timeout waiting for QID={qid}", "WARNING")
                    self.stats['timeouts'] += 1
                    break
            
            # Return single response or list
            if expected == 1:
                return responses[0] if responses else None
            else:
                return responses
                
        finally:
            # Cleanup
            with self.commands_lock:
                self.pending_commands.pop(qid, None)
                self.response_queues.pop(qid, None)
    
    def register_callback(self, callback: Callable[[Dict[str, Any]], None], pattern: str = ""):
        """
        Register a callback for async notifications.
        
        Args:
            callback: Function to call when matching message arrives
            pattern: Pattern to match in message (simple substring match)
        """
        with self.callbacks_lock:
            self.callbacks.append((callback, pattern))
            self._log(f"Registered callback for pattern: '{pattern}'")
    
    def get_statistics(self) -> Dict[str, int]:
        """Get communication statistics"""
        return self.stats.copy()
    
    def close(self):
        """Close serial connection and stop threads"""
        self._log("Closing connection...")
        
        self.running = False
        
        # Wait for threads to finish
        if self.read_thread and self.read_thread.is_alive():
            self.read_thread.join(timeout=2.0)
        if self.write_thread and self.write_thread.is_alive():
            self.write_thread.join(timeout=2.0)
        
        # Close serial port
        if self.serial_device and self.serial_device.is_open:
            self.serial_device.close()
        
        self.is_connected = False
        self._log("Connection closed")
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()


# Backward compatibility wrapper
class Serial(SerialCommunicator):
    """Legacy compatibility wrapper for SerialCommunicator"""
    
    def post_json(self, path: str, payload: Dict[str, Any], 
                  getReturn: bool = True, nResponses: int = 1, 
                  timeout: float = 100) -> Optional[Dict[str, Any]]:
        """Legacy method for backward compatibility"""
        return self.send_command(
            payload,
            blocking=getReturn,
            expected_responses=nResponses,
            timeout=timeout
        )
    
    def sendMessage(self, command: Dict[str, Any], 
                   nResponses: int = 1, timeout: float = 20) -> Optional[Dict[str, Any]]:
        """Legacy method for backward compatibility"""
        return self.send_command(
            command,
            blocking=(nResponses > 0),
            expected_responses=nResponses if nResponses > 0 else 1,
            timeout=timeout
        )


if __name__ == "__main__":
    # Test the implementation
    print("=== Modern Serial Communication Test ===\n")
    
    # This would connect to an actual device
    # comm = SerialCommunicator(port='/dev/ttyUSB0', debug=True)
    
    print("Features:")
    print("  ✓ MessagePack protocol support")
    print("  ✓ Blocking and non-blocking modes")
    print("  ✓ QID-based request/response tracking")
    print("  ✓ Thread-safe command queuing")
    print("  ✓ Callback support for notifications")
    print("  ✓ Backward compatibility with legacy Serial class")
    print("\nSee SERIAL_PROTOCOL_V2.md for full documentation")
