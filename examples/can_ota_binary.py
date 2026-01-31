#!/usr/bin/env python3
"""
CAN OTA Binary Streaming Protocol
===================================
High-performance firmware update over CAN bus using binary streaming.

Protocol Design:
- JSON commands for control (START, END, VERIFY, ABORT, STATUS)
- Binary streaming for DATA chunks (no JSON/Base64 overhead)
- Proper flow control with ACK waiting
- Automatic baudrate switching for optimal performance

Binary Packet Format for DATA:
  [SYNC_1][SYNC_2][CMD][CHUNK_H][CHUNK_L][SIZE_H][SIZE_L][CRC32_4bytes][DATA...][CHECKSUM]
  
  - SYNC_1, SYNC_2: 0xAA, 0x55 (magic bytes for frame sync)
  - CMD: 0x63 (OTA_CAN_DATA)
  - CHUNK_H, CHUNK_L: 16-bit chunk index (big endian)
  - SIZE_H, SIZE_L: 16-bit data size (big endian)
  - CRC32: 32-bit CRC of DATA (little endian)
  - DATA: raw firmware chunk bytes
  - CHECKSUM: XOR of all bytes from SYNC_1 to last DATA byte

Author: UC2 Team
"""

import serial
import time
import json
import hashlib
import struct
import zlib
from pathlib import Path
from typing import Dict, Any, Optional, Callable
from dataclasses import dataclass
from enum import IntEnum

# Protocol constants
SYNC_BYTE_1 = 0xAA
SYNC_BYTE_2 = 0x55
OTA_CMD_DATA = 0x63
OTA_CMD_ACK = 0x66
OTA_CMD_NAK = 0x67

# Configuration
DEFAULT_CHUNK_SIZE = 1024  # Smaller chunks = more reliable, reduced to 1KB
CONTROL_BAUDRATE = 115200  # Baudrate for JSON control commands
DATA_BAUDRATE = 921600     # Baudrate for binary data streaming
DEFAULT_TIMEOUT = 5.0
MAX_RETRIES = 5
ACK_TIMEOUT = 2.0  # Timeout waiting for ACK per chunk


class OTAStatus(IntEnum):
    """OTA status codes matching C++ enum"""
    OK = 0x00
    ERR_BUSY = 0x01
    ERR_INVALID_SIZE = 0x02
    ERR_PARTITION = 0x03
    ERR_BEGIN = 0x04
    ERR_WRITE = 0x05
    ERR_CRC = 0x06
    ERR_SEQUENCE = 0x07
    ERR_MD5 = 0x08
    ERR_END = 0x09
    ERR_TIMEOUT = 0x0A
    ERR_ABORTED = 0x0B
    ERR_MEMORY = 0x0C
    ERR_NOT_STARTED = 0x0D


@dataclass
class OTAProgress:
    """Progress information for callbacks"""
    bytes_sent: int
    bytes_total: int
    chunks_sent: int
    chunks_total: int
    elapsed_seconds: float
    
    @property
    def percent(self) -> float:
        return (self.bytes_sent / self.bytes_total * 100) if self.bytes_total > 0 else 0
    
    @property
    def speed_kbps(self) -> float:
        return (self.bytes_sent / 1024 / self.elapsed_seconds) if self.elapsed_seconds > 0 else 0
    
    @property
    def eta_seconds(self) -> float:
        if self.bytes_sent == 0:
            return 0
        return self.elapsed_seconds / self.bytes_sent * (self.bytes_total - self.bytes_sent)


def calculate_crc32(data: bytes) -> int:
    """Calculate CRC32 for chunk data."""
    return zlib.crc32(data) & 0xFFFFFFFF


def calculate_md5(data: bytes) -> str:
    """Calculate MD5 hash of firmware."""
    return hashlib.md5(data).hexdigest()


def calculate_checksum(data: bytes) -> int:
    """Calculate XOR checksum of all bytes."""
    checksum = 0
    for b in data:
        checksum ^= b
    return checksum


def build_binary_packet(chunk_index: int, data: bytes) -> bytes:
    """
    Build binary packet for OTA data chunk.
    
    Format: [0xAA][0x55][0x63][CHUNK_H][CHUNK_L][SIZE_H][SIZE_L][CRC32_LE][DATA][CHECKSUM]
    """
    size = len(data)
    crc32 = calculate_crc32(data)
    
    # Build header
    header = struct.pack('>BBHH', 
                         OTA_CMD_DATA,      # Command byte
                         0x00,              # Reserved
                         chunk_index,       # Chunk index (big endian)
                         size)              # Size (big endian)
    
    # CRC32 in little endian (matches ESP32)
    crc_bytes = struct.pack('<I', crc32)
    
    # Combine: sync + header + crc + data
    packet_body = bytes([SYNC_BYTE_1, SYNC_BYTE_2]) + header + crc_bytes + data
    
    # Calculate checksum over entire packet body
    checksum = calculate_checksum(packet_body)
    
    # Final packet
    return packet_body + bytes([checksum])


def is_json_response_success(response: Optional[Dict]) -> bool:
    """Check if JSON response indicates success."""
    if not isinstance(response, dict):
        return False
    
    success = response.get('success', False)
    if success is True or success == 'true' or success == 1 or success == '1':
        return True
    
    if 'error' in response:
        return False
    
    return False


def get_json_response_error(response: Optional[Dict]) -> str:
    """Extract error message from JSON response."""
    if not isinstance(response, dict):
        return str(response) if response else "No response"
    return response.get('error', response.get('message', 'Unknown error'))


class CANOTABinary:
    """
    High-performance CAN OTA updater using binary streaming.
    
    Features:
    - Binary data transfer (no JSON/Base64 overhead for chunks)
    - Proper flow control with ACK waiting
    - Automatic retry with exponential backoff
    - Progress callbacks
    """
    
    def __init__(self, port: str, control_baudrate: int = CONTROL_BAUDRATE,
                 data_baudrate: int = DATA_BAUDRATE, chunk_size: int = DEFAULT_CHUNK_SIZE):
        """
        Initialize CAN OTA Binary updater.
        
        Args:
            port: Serial port path
            control_baudrate: Baudrate for JSON control commands
            data_baudrate: Baudrate for binary data streaming
            chunk_size: Size of each firmware chunk
        """
        self.port = port
        self.control_baudrate = control_baudrate
        self.data_baudrate = data_baudrate
        self.chunk_size = chunk_size
        self.ser: Optional[serial.Serial] = None
        self._abort_requested = False
        self._progress_callback: Optional[Callable[[OTAProgress], None]] = None
    
    def set_progress_callback(self, callback: Callable[[OTAProgress], None]) -> None:
        """Set callback for progress updates."""
        self._progress_callback = callback
    
    def abort(self) -> None:
        """Request abort of current upload."""
        self._abort_requested = True
    
    def _open_serial(self, baudrate: int) -> bool:
        """Open or reconfigure serial port."""
        try:
            if self.ser is not None and self.ser.is_open:
                if self.ser.baudrate != baudrate:
                    self.ser.baudrate = baudrate
                    time.sleep(0.1)  # Allow reconfiguration
                return True
            
            self.ser = serial.Serial(
                port=self.port,
                baudrate=baudrate,
                timeout=1.0,
                write_timeout=2.0
            )
            time.sleep(0.3)  # Allow port to stabilize
            return True
        except serial.SerialException as e:
            print(f"Serial error: {e}")
            return False
    
    def _close_serial(self) -> None:
        """Close serial port."""
        if self.ser is not None and self.ser.is_open:
            self.ser.close()
            self.ser = None
    
    def _send_json_command(self, canid: int, action: str, timeout: float = DEFAULT_TIMEOUT,
                           **kwargs) -> Optional[Dict]:
        """Send JSON control command and wait for response."""
        command = {
            "task": "/can_ota",
            "canid": canid,
            "action": action,
            **kwargs
        }
        
        json_str = json.dumps(command, separators=(',', ':'))
        
        # Clear buffers
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()
        
        # Send command
        print(f"  TX JSON: {action}")
        self.ser.write((json_str + '\n').encode('utf-8'))
        self.ser.flush()
        
        # Wait for JSON response
        start_time = time.time()
        buffer = ""
        
        while (time.time() - start_time) < timeout:
            if self.ser.in_waiting > 0:
                try:
                    char = self.ser.read(1).decode('utf-8', errors='ignore')
                    buffer += char
                    
                    # Look for complete JSON
                    if '{' in buffer and '}' in buffer:
                        start = buffer.find('{')
                        end = buffer.rfind('}') + 1
                        json_str = buffer[start:end]
                        try:
                            response = json.loads(json_str)
                            print(f"  RX JSON: {response}")
                            return response
                        except json.JSONDecodeError:
                            pass
                except:
                    pass
            else:
                time.sleep(0.01)
        
        print(f"  Timeout waiting for {action} response")
        return None
    
    def _send_binary_chunk(self, chunk_index: int, data: bytes) -> bool:
        """
        Send binary chunk and wait for ACK.
        
        Returns True if ACK received, False otherwise.
        """
        packet = build_binary_packet(chunk_index, data)
        
        # Clear input buffer
        self.ser.reset_input_buffer()
        
        # Send packet
        self.ser.write(packet)
        self.ser.flush()
        
        # Wait for ACK/NAK response
        # The Master will respond with JSON after relaying to slave
        start_time = time.time()
        buffer = ""
        
        while (time.time() - start_time) < ACK_TIMEOUT:
            if self.ser.in_waiting > 0:
                try:
                    char = self.ser.read(1).decode('utf-8', errors='ignore')
                    buffer += char
                    
                    # Look for JSON response
                    if '{' in buffer and '}' in buffer:
                        start = buffer.find('{')
                        end = buffer.rfind('}') + 1
                        json_str = buffer[start:end]
                        try:
                            response = json.loads(json_str)
                            if is_json_response_success(response):
                                return True
                            else:
                                print(f"    NAK: {get_json_response_error(response)}")
                                return False
                        except json.JSONDecodeError:
                            pass
                except:
                    pass
            else:
                time.sleep(0.005)
        
        print(f"    Timeout waiting for chunk {chunk_index} ACK")
        return False
    
    def _send_json_chunk(self, canid: int, chunk_index: int, data: bytes, 
                         timeout: float = DEFAULT_TIMEOUT) -> bool:
        """
        Send chunk via JSON (fallback, slower but more compatible).
        Uses smaller chunk size and hex encoding instead of base64.
        """
        crc32 = calculate_crc32(data)
        
        # Use hex encoding - more reliable than base64 for JSON
        hex_data = data.hex()
        
        command = {
            "task": "/can_ota",
            "canid": canid,
            "action": "data",
            "chunk_index": chunk_index,
            "chunk_size": len(data),
            "chunk_crc": crc32,
            "hex": hex_data  # Use hex instead of base64
        }
        
        json_str = json.dumps(command, separators=(',', ':'))
        
        # Clear buffers
        self.ser.reset_input_buffer()
        
        # Send
        self.ser.write((json_str + '\n').encode('utf-8'))
        self.ser.flush()
        
        # Wait for response
        start_time = time.time()
        buffer = ""
        
        while (time.time() - start_time) < timeout:
            if self.ser.in_waiting > 0:
                try:
                    char = self.ser.read(1).decode('utf-8', errors='ignore')
                    buffer += char
                    
                    if '{' in buffer and '}' in buffer:
                        start = buffer.find('{')
                        end = buffer.rfind('}') + 1
                        json_str = buffer[start:end]
                        try:
                            response = json.loads(json_str)
                            return is_json_response_success(response)
                        except json.JSONDecodeError:
                            pass
                except:
                    pass
            else:
                time.sleep(0.005)
        
        return False
    
    def upload_firmware(self, canid: int, firmware_path: str,
                        progress_callback: Optional[Callable[[OTAProgress], None]] = None,
                        use_binary: bool = False) -> bool:
        """
        Upload firmware to CAN device.
        
        Args:
            canid: Target CAN device ID
            firmware_path: Path to firmware binary file
            progress_callback: Optional callback for progress updates
            use_binary: Use binary protocol (faster) vs JSON (more compatible)
            
        Returns:
            True if upload successful
        """
        if progress_callback:
            self._progress_callback = progress_callback
        
        self._abort_requested = False
        firmware_path = Path(firmware_path)
        
        # Validate firmware
        if not firmware_path.exists():
            print(f"Error: Firmware file not found: {firmware_path}")
            return False
        
        firmware_data = firmware_path.read_bytes()
        firmware_size = len(firmware_data)
        
        if firmware_size == 0:
            print("Error: Firmware file is empty")
            return False
        
        # Calculate metadata
        md5_hash = calculate_md5(firmware_data)
        num_chunks = (firmware_size + self.chunk_size - 1) // self.chunk_size
        
        print("=" * 60)
        print("CAN OTA Binary Streaming Upload")
        print("=" * 60)
        print(f"Serial Port: {self.port}")
        print(f"Target CAN ID: {canid}")
        print(f"Firmware: {firmware_path.name}")
        print(f"Size: {firmware_size:,} bytes ({num_chunks} chunks of {self.chunk_size} bytes)")
        print(f"MD5: {md5_hash}")
        print(f"Mode: {'Binary' if use_binary else 'JSON'}")
        print("=" * 60)
        print()
        
        try:
            # Open serial at control baudrate
            print(f"Opening serial port at {self.control_baudrate} baud...")
            if not self._open_serial(self.control_baudrate):
                return False
            
            # Step 1: Send START command
            print("\n[1/4] Sending START command...")
            response = self._send_json_command(
                canid, "start",
                firmware_size=firmware_size,
                chunk_size=self.chunk_size,
                total_chunks=num_chunks,
                md5=md5_hash,
                timeout=10.0
            )
            
            if not is_json_response_success(response):
                error = get_json_response_error(response)
                print(f"  START failed: {error}")
                self._close_serial()
                return False
            
            print("  Device ready!")
            
            # Add delay to ensure slave is ready
            time.sleep(0.5)
            
            # Step 2: Upload chunks
            print(f"\n[2/4] Uploading {num_chunks} chunks...")
            start_time = time.time()
            
            # Switch to data baudrate if using binary mode
            if use_binary and self.data_baudrate != self.control_baudrate:
                print(f"  Switching to {self.data_baudrate} baud for data transfer...")
                self._open_serial(self.data_baudrate)
                time.sleep(0.2)
            
            for chunk_idx in range(num_chunks):
                if self._abort_requested:
                    print("\n  Abort requested")
                    self._send_json_command(canid, "abort", timeout=2.0)
                    self._close_serial()
                    return False
                
                # Extract chunk
                chunk_start = chunk_idx * self.chunk_size
                chunk_end = min(chunk_start + self.chunk_size, firmware_size)
                chunk_data = firmware_data[chunk_start:chunk_end]
                
                # Send with retry
                success = False
                for retry in range(MAX_RETRIES):
                    if use_binary:
                        success = self._send_binary_chunk(chunk_idx, chunk_data)
                    else:
                        success = self._send_json_chunk(canid, chunk_idx, chunk_data)
                    
                    if success:
                        break
                    
                    if retry < MAX_RETRIES - 1:
                        print(f"    Chunk {chunk_idx} retry {retry + 1}/{MAX_RETRIES}")
                        time.sleep(0.1 * (retry + 1))  # Exponential backoff
                
                if not success:
                    print(f"\n  Chunk {chunk_idx} failed after {MAX_RETRIES} retries")
                    self._send_json_command(canid, "abort", timeout=2.0)
                    self._close_serial()
                    return False
                
                # Progress update
                elapsed = time.time() - start_time
                progress = OTAProgress(
                    bytes_sent=chunk_end,
                    bytes_total=firmware_size,
                    chunks_sent=chunk_idx + 1,
                    chunks_total=num_chunks,
                    elapsed_seconds=elapsed
                )
                
                if self._progress_callback:
                    self._progress_callback(progress)
                
                # Print progress every 10% or 50 chunks
                if (chunk_idx + 1) % max(1, num_chunks // 10) == 0 or chunk_idx == num_chunks - 1:
                    print(f"  Progress: {chunk_idx + 1}/{num_chunks} ({progress.percent:.1f}%) | "
                          f"Speed: {progress.speed_kbps:.1f} KB/s | ETA: {progress.eta_seconds:.0f}s")
            
            elapsed = time.time() - start_time
            avg_speed = firmware_size / 1024 / elapsed if elapsed > 0 else 0
            print(f"  Upload complete: {elapsed:.1f}s, {avg_speed:.1f} KB/s")
            
            # Switch back to control baudrate
            if use_binary and self.data_baudrate != self.control_baudrate:
                print(f"  Switching back to {self.control_baudrate} baud...")
                self._open_serial(self.control_baudrate)
                time.sleep(0.2)
            
            # Step 3: Send END command
            print("\n[3/4] Finalizing OTA...")
            response = self._send_json_command(canid, "end", timeout=10.0)
            
            if not is_json_response_success(response):
                print(f"  END failed: {get_json_response_error(response)}")
                self._send_json_command(canid, "abort", timeout=2.0)
                self._close_serial()
                return False
            
            print("  OTA finalized")
            
            # Step 4: Verify MD5
            print("\n[4/4] Verifying firmware...")
            response = self._send_json_command(canid, "verify", md5=md5_hash, timeout=15.0)
            
            if not is_json_response_success(response):
                print(f"  Verification failed: {get_json_response_error(response)}")
                self._close_serial()
                return False
            
            print("  Verification successful!")
            
            print("\n" + "=" * 60)
            print("OTA UPDATE COMPLETED SUCCESSFULLY!")
            print("=" * 60)
            print(f"Total time: {elapsed:.1f}s")
            print(f"Average speed: {avg_speed:.1f} KB/s")
            print("\nDevice will reboot with new firmware...")
            
            self._close_serial()
            return True
            
        except Exception as e:
            print(f"\nError: {e}")
            import traceback
            traceback.print_exc()
            try:
                self._send_json_command(canid, "abort", timeout=2.0)
                self._close_serial()
            except:
                pass
            return False


def main():
    """Main entry point for command line usage."""
    import argparse
    
    parser = argparse.ArgumentParser(description="CAN OTA Binary Firmware Upload")
    parser.add_argument("--port", "-p", default="/dev/cu.SLAB_USBtoUART",
                        help="Serial port")
    parser.add_argument("--canid", "-c", type=int, default=11,
                        help="Target CAN ID")
    parser.add_argument("--firmware", "-f", default="/Users/bene/Dropbox/Dokumente/Promotion/PROJECTS/UC2-REST/binaries/latest/esp32_seeed_xiao_esp32s3_can_slave_motor.bin",
                        help="Firmware binary file")
    parser.add_argument("--chunk-size", type=int, default=DEFAULT_CHUNK_SIZE,
                        help=f"Chunk size (default: {DEFAULT_CHUNK_SIZE})")
    parser.add_argument("--baudrate", "-b", type=int, default=CONTROL_BAUDRATE,
                        help=f"Control baudrate (default: {CONTROL_BAUDRATE})")
    parser.add_argument("--binary", default="true", action="store_true",
                        help="Use binary protocol (experimental)")
    
    args = parser.parse_args()
    
    ota = CANOTABinary(
        port=args.port,
        control_baudrate=args.baudrate,
        chunk_size=args.chunk_size
    )
    
    success = ota.upload_firmware(
        canid=args.canid,
        firmware_path=args.firmware,
        use_binary=args.binary
    )
    
    exit(0 if success else 1)


if __name__ == "__main__":
    main()
