#!/usr/bin/env python3
"""
CAN OTA Binary Streaming Protocol - High Performance Version
=============================================================
Bypasses JSON for data transfer, using raw binary packets at 2Mbaud.

Protocol Flow:
1. JSON START at 115200 baud → Slave prepared, Master responds with binary mode info
2. JSON BINARY_START → Master switches to 2Mbaud and enters binary mode
3. Python switches to 2Mbaud
4. Binary DATA packets sent at high speed
5. Binary ACK/NAK responses for flow control  
6. Binary END packet
7. Switch back to 115200 baud
8. JSON VERIFY/END for finalization

Binary Packet Format (sent by Python):
  [0xAA][0x55][CMD][CHUNK_H][CHUNK_L][SIZE_H][SIZE_L][CRC32_LE(4)][DATA...][CHECKSUM]

Binary Response Format (received from ESP32):
  [0xAA][0x55][CMD][STATUS][CHUNK_H][CHUNK_L][CHECKSUM]
  CMD: 0x06=ACK, 0x07=NAK

Author: UC2 Team
"""

import serial
import time
import json
import struct
import zlib
import hashlib
from pathlib import Path
from typing import Optional, Tuple, Callable
from dataclasses import dataclass
from enum import IntEnum

# Protocol constants
SYNC_1 = 0xAA
SYNC_2 = 0x55

# Commands
CMD_DATA = 0x01
CMD_ACK = 0x06
CMD_NAK = 0x07
CMD_END = 0x0F
CMD_ABORT = 0x10
CMD_SWITCH_BAUD = 0x11

# Baudrates
NORMAL_BAUDRATE = 921600
HIGH_BAUDRATE = 921600  # 2 Mbaud

# Timeouts
ACK_TIMEOUT = 2.0  # Seconds to wait for ACK
PACKET_TIMEOUT = 0.5  # Seconds to wait for complete response packet

# Chunk size - must match ESP32 BINARY_OTA_MAX_CHUNK_SIZE
CHUNK_SIZE = 1024

# Max retries per chunk
MAX_RETRIES = 5


class OTAStatus(IntEnum):
    OK = 0x00
    ERR_SYNC = 0x01
    ERR_CHECKSUM = 0x02
    ERR_CRC = 0x03
    ERR_SEQUENCE = 0x04
    ERR_SIZE = 0x05
    ERR_TIMEOUT = 0x06
    ERR_RELAY = 0x07
    ERR_SLAVE_NAK = 0x08


@dataclass
class OTAProgress:
    """Progress information"""
    bytes_sent: int
    bytes_total: int
    chunks_sent: int
    chunks_total: int
    elapsed_seconds: float
    retries: int = 0
    
    @property
    def percent(self) -> float:
        return (self.bytes_sent / self.bytes_total * 100) if self.bytes_total > 0 else 0
    
    @property
    def speed_kbps(self) -> float:
        return (self.bytes_sent / 1024 / self.elapsed_seconds) if self.elapsed_seconds > 0 else 0


def calculate_crc32(data: bytes) -> int:
    """Calculate CRC32 (same as ESP32 esp_crc32_le)"""
    return zlib.crc32(data) & 0xFFFFFFFF


def calculate_checksum(data: bytes) -> int:
    """Calculate XOR checksum"""
    checksum = 0
    for b in data:
        checksum ^= b
    return checksum


def calculate_md5(data: bytes) -> str:
    """Calculate MD5 hash"""
    return hashlib.md5(data).hexdigest()


def build_data_packet(chunk_index: int, data: bytes) -> bytes:
    """
    Build binary data packet.
    
    Format: [0xAA][0x55][0x01][CHUNK_H][CHUNK_L][SIZE_H][SIZE_L][CRC32_LE(4)][DATA][CHECKSUM]
    """
    crc32 = calculate_crc32(data)
    size = len(data)
    
    # Build packet without checksum
    header = struct.pack('>BBHH', 
                         CMD_DATA,      # Command
                         0x00,          # Reserved (will be overwritten)
                         chunk_index,   # Chunk index (big endian)
                         size)          # Size (big endian)
    
    # Fix: pack header correctly
    header = bytes([SYNC_1, SYNC_2, CMD_DATA]) + struct.pack('>HH', chunk_index, size)
    
    # CRC32 in little endian
    crc_bytes = struct.pack('<I', crc32)
    
    # Combine
    packet_body = header + crc_bytes + data
    
    # Calculate checksum
    checksum = calculate_checksum(packet_body)
    
    return packet_body + bytes([checksum])


def build_end_packet() -> bytes:
    """Build END packet to signal transfer complete"""
    header = bytes([SYNC_1, SYNC_2, CMD_END, 0x00, 0x00, 0x00, 0x00])
    # Add dummy CRC
    header += bytes([0x00, 0x00, 0x00, 0x00])
    checksum = calculate_checksum(header)
    return header + bytes([checksum])


def build_switch_baud_packet() -> bytes:
    """Build packet to switch back to normal baudrate"""
    header = bytes([SYNC_1, SYNC_2, CMD_SWITCH_BAUD, 0x00, 0x00, 0x00, 0x00])
    header += bytes([0x00, 0x00, 0x00, 0x00])  # Dummy CRC
    checksum = calculate_checksum(header)
    return header + bytes([checksum])


class BinaryOTAUploader:
    """High-performance binary OTA uploader"""
    
    def __init__(self, port: str, normal_baud: int = NORMAL_BAUDRATE, 
                 high_baud: int = HIGH_BAUDRATE, chunk_size: int = CHUNK_SIZE):
        self.port = port
        self.normal_baud = normal_baud
        self.high_baud = high_baud
        self.chunk_size = chunk_size
        self.ser: Optional[serial.Serial] = None
        self._abort = False
        self._in_binary_mode = False
        self._progress_callback: Optional[Callable[[OTAProgress], None]] = None
    
    def set_progress_callback(self, callback: Callable[[OTAProgress], None]):
        self._progress_callback = callback
    
    def abort(self):
        self._abort = True
    
    def _open_serial(self, baudrate: int) -> bool:
        """Open or reconfigure serial port"""
        try:
            if self.ser and self.ser.is_open:
                if self.ser.baudrate != baudrate:
                    print(f"  Switching baudrate: {self.ser.baudrate} → {baudrate}")
                    self.ser.close()
                    time.sleep(0.1)
                    self.ser = serial.Serial(
                        port=self.port,
                        baudrate=baudrate,
                        timeout=0.1,
                        write_timeout=2.0
                    )
                    time.sleep(0.1)
                return True
            
            self.ser = serial.Serial(
                port=self.port,
                baudrate=baudrate,
                timeout=0.1,
                write_timeout=2.0
            )
            time.sleep(0.2)
            return True
        except serial.SerialException as e:
            print(f"Serial error: {e}")
            return False
    
    def _close_serial(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            self.ser = None
    
    def _send_json(self, command: dict, timeout: float = 5.0) -> Optional[dict]:
        """Send JSON command and wait for JSON response"""
        json_str = json.dumps(command, separators=(',', ':'))
        
        self.ser.reset_input_buffer()
        self.ser.write((json_str + '\n').encode())
        self.ser.flush()
        
        # Wait for JSON response
        start = time.time()
        buffer = ""
        
        while time.time() - start < timeout:
            if self.ser.in_waiting:
                try:
                    char = self.ser.read(1).decode('utf-8', errors='ignore')
                    buffer += char
                    
                    if '{' in buffer and '}' in buffer:
                        start_idx = buffer.find('{')
                        end_idx = buffer.rfind('}') + 1
                        try:
                            return json.loads(buffer[start_idx:end_idx])
                        except json.JSONDecodeError:
                            pass
                except:
                    pass
            else:
                time.sleep(0.01)
        
        return None
    
    def _wait_for_binary_ack(self, timeout: float = ACK_TIMEOUT) -> Tuple[bool, int, int]:
        """
        Wait for binary ACK/NAK response.
        
        Returns: (success, status, chunk_index)
        """
        start = time.time()
        buffer = bytearray()
        
        while time.time() - start < timeout:
            if self.ser.in_waiting:
                buffer.extend(self.ser.read(self.ser.in_waiting))
                
                # Look for sync sequence
                while len(buffer) >= 2:
                    if buffer[0] == SYNC_1 and buffer[1] == SYNC_2:
                        # Found sync - check if we have complete response (7 bytes)
                        if len(buffer) >= 7:
                            cmd = buffer[2]
                            status = buffer[3]
                            chunk_index = (buffer[4] << 8) | buffer[5]
                            checksum = buffer[6]
                            
                            # Verify checksum
                            calc_checksum = calculate_checksum(bytes(buffer[:6]))
                            if calc_checksum != checksum:
                                print(f"    Checksum error: got 0x{checksum:02X}, expected 0x{calc_checksum:02X}")
                                buffer = buffer[1:]  # Skip one byte and retry
                                continue
                            
                            if cmd == CMD_ACK:
                                return (True, status, chunk_index)
                            elif cmd == CMD_NAK:
                                return (False, status, chunk_index)
                            else:
                                print(f"    Unknown response cmd: 0x{cmd:02X}")
                                buffer = buffer[7:]  # Skip this response
                                continue
                        else:
                            break  # Wait for more data
                    else:
                        buffer = buffer[1:]  # Skip non-sync byte
            else:
                time.sleep(0.001)
        
        return (False, OTAStatus.ERR_TIMEOUT, 0)
    
    def _send_binary_chunk(self, chunk_index: int, data: bytes) -> bool:
        """Send binary chunk and wait for ACK"""
        packet = build_data_packet(chunk_index, data)
        
        for retry in range(MAX_RETRIES):
            self.ser.reset_input_buffer()
            self.ser.write(packet)
            self.ser.flush()
            
            success, status, resp_chunk = self._wait_for_binary_ack()
            
            if success and status == OTAStatus.OK:
                return True
            
            if not success:
                if status == OTAStatus.ERR_SEQUENCE:
                    print(f"    Sequence error: ESP expects chunk {resp_chunk}, we sent {chunk_index}")
                elif status == OTAStatus.ERR_TIMEOUT:
                    print(f"    Timeout waiting for ACK (retry {retry + 1}/{MAX_RETRIES})")
                else:
                    print(f"    NAK received: status={status}, chunk={resp_chunk}")
                
                time.sleep(0.05 * (retry + 1))  # Exponential backoff
            else:
                print(f"    Unexpected status: {status}")
        
        return False
    
    def upload_firmware(self, canid: int, firmware_path: str,
                        progress_callback: Optional[Callable[[OTAProgress], None]] = None) -> bool:
        """
        Upload firmware using binary protocol.
        
        Args:
            canid: Target CAN device ID
            firmware_path: Path to firmware binary
            progress_callback: Optional progress callback
        """
        if progress_callback:
            self._progress_callback = progress_callback
        
        self._abort = False
        firmware_path = Path(firmware_path)
        
        # Validate firmware
        if not firmware_path.exists():
            print(f"Error: Firmware not found: {firmware_path}")
            return False
        
        firmware_data = firmware_path.read_bytes()
        firmware_size = len(firmware_data)
        
        if firmware_size == 0:
            print("Error: Firmware is empty")
            return False
        
        md5_hash = calculate_md5(firmware_data)
        num_chunks = (firmware_size + self.chunk_size - 1) // self.chunk_size
        
        print("=" * 70)
        print("CAN OTA Binary Protocol - High Performance Upload")
        print("=" * 70)
        print(f"Port: {self.port}")
        print(f"CAN ID: {canid}")
        print(f"Firmware: {firmware_path.name}")
        print(f"Size: {firmware_size:,} bytes ({num_chunks} chunks of {self.chunk_size} bytes)")
        print(f"MD5: {md5_hash}")
        print(f"Normal Baud: {self.normal_baud}, High Baud: {self.high_baud}")
        print("=" * 70)
        
        total_retries = 0
        
        try:
            # Step 1: Open at normal baudrate
            print(f"\n[1/6] Connecting at {self.normal_baud} baud...")
            if not self._open_serial(self.normal_baud):
                return False
            
            # Step 2: Send JSON START command
            print("\n[2/6] Sending START command...")
            response = self._send_json({
                "task": "/can_ota",
                "canid": canid,
                "action": "start",
                "firmware_size": firmware_size,
                "total_chunks": num_chunks,
                "chunk_size": self.chunk_size,
                "md5": md5_hash,
                "binary_mode": True  # Request binary mode
            }, timeout=10.0)
            
            if not response:
                print("  No response from START command")
                return False
            
            if not response.get('success'):
                print(f"  START failed: {response.get('error', 'Unknown error')}")
                return False
            
            print("  Slave initialized for OTA")
            time.sleep(0.3)
            
            # Step 3: Send BINARY_START to switch to binary mode
            print(f"\n[3/6] Switching to binary mode at {self.high_baud} baud...")
            response = self._send_json({
                "task": "/can_ota",
                "canid": canid,
                "action": "binary_start"
            }, timeout=5.0)
            
            # Note: After binary_start, ESP32 will switch baudrate
            # We may or may not get a response depending on timing
            time.sleep(0.2)
            
            # Switch Python to high baudrate
            if not self._open_serial(self.high_baud):
                print("  Failed to switch to high baudrate")
                return False
            
            self._in_binary_mode = True
            time.sleep(0.2)
            
            # Wait for ready ACK from ESP32
            print("  Waiting for binary mode ACK...")
            success, status, _ = self._wait_for_binary_ack(timeout=3.0)
            if not success:
                print(f"  No ACK received (status={status}), trying anyway...")
            else:
                print("  Binary mode active!")
            
            # Step 4: Upload chunks
            print(f"\n[4/6] Uploading {num_chunks} chunks...")
            start_time = time.time()
            
            for chunk_idx in range(num_chunks):
                if self._abort:
                    print("\n  Abort requested!")
                    break
                
                # Extract chunk
                chunk_start = chunk_idx * self.chunk_size
                chunk_end = min(chunk_start + self.chunk_size, firmware_size)
                chunk_data = firmware_data[chunk_start:chunk_end]
                
                # Send with retry
                if not self._send_binary_chunk(chunk_idx, chunk_data):
                    print(f"\n  Failed to send chunk {chunk_idx} after {MAX_RETRIES} retries")
                    self._send_abort()
                    return False
                
                # Progress
                elapsed = time.time() - start_time
                progress = OTAProgress(
                    bytes_sent=chunk_end,
                    bytes_total=firmware_size,
                    chunks_sent=chunk_idx + 1,
                    chunks_total=num_chunks,
                    elapsed_seconds=elapsed,
                    retries=total_retries
                )
                
                if self._progress_callback:
                    self._progress_callback(progress)
                
                # Print progress
                if (chunk_idx + 1) % max(1, num_chunks // 20) == 0 or chunk_idx == num_chunks - 1:
                    print(f"  [{chunk_idx + 1}/{num_chunks}] {progress.percent:.1f}% | "
                          f"{progress.speed_kbps:.1f} KB/s")
            
            if self._abort:
                self._send_abort()
                return False
            
            elapsed = time.time() - start_time
            speed = firmware_size / 1024 / elapsed if elapsed > 0 else 0
            print(f"\n  Upload complete: {elapsed:.1f}s, {speed:.1f} KB/s average")
            
            # Step 5: Send END packet and switch back to normal baudrate
            print(f"\n[5/6] Finalizing and switching back to {self.normal_baud} baud...")
            
            # Send END packet
            end_packet = build_end_packet()
            self.ser.write(end_packet)
            self.ser.flush()
            time.sleep(0.1)
            
            # Send switch baudrate command
            switch_packet = build_switch_baud_packet()
            self.ser.write(switch_packet)
            self.ser.flush()
            time.sleep(0.2)
            
            # Switch Python back to normal baudrate
            self._in_binary_mode = False
            if not self._open_serial(self.normal_baud):
                print("  Warning: Failed to switch back to normal baudrate")
            
            time.sleep(0.3)
            
            # Step 6: Verify
            print("\n[6/6] Verifying firmware...")
            response = self._send_json({
                "task": "/can_ota",
                "canid": canid,
                "action": "verify",
                "md5": md5_hash
            }, timeout=15.0)
            
            if response and response.get('success'):
                print("  Verification successful!")
            else:
                print(f"  Verification response: {response}")
            
            # Send finish/end command
            response = self._send_json({
                "task": "/can_ota",
                "canid": canid,
                "action": "end"
            }, timeout=10.0)
            
            print("\n" + "=" * 70)
            print("OTA UPLOAD COMPLETED SUCCESSFULLY!")
            print("=" * 70)
            print(f"Total time: {time.time() - start_time:.1f}s")
            print(f"Average speed: {speed:.1f} KB/s")
            print(f"Retries: {total_retries}")
            
            self._close_serial()
            return True
            
        except Exception as e:
            print(f"\nError: {e}")
            import traceback
            traceback.print_exc()
            self._send_abort()
            return False
        finally:
            if self._in_binary_mode:
                # Try to exit binary mode
                try:
                    self._open_serial(self.normal_baud)
                except:
                    pass
    
    def _send_abort(self):
        """Send abort command"""
        try:
            if self._in_binary_mode:
                packet = bytes([SYNC_1, SYNC_2, CMD_ABORT, 0, 0, 0, 0, 0, 0, 0, 0])
                checksum = calculate_checksum(packet)
                self.ser.write(packet + bytes([checksum]))
                self.ser.flush()
                time.sleep(0.2)
                self._open_serial(self.normal_baud)
            
            self._send_json({
                "task": "/can_ota",
                "canid": 11,  # Default
                "action": "abort"
            }, timeout=2.0)
        except:
            pass
        self._close_serial()


def main():
    """Command line interface"""
    import argparse
    
    parser = argparse.ArgumentParser(description="CAN OTA Binary Protocol Upload")
    parser.add_argument("--port", "-p", default="/dev/cu.SLAB_USBtoUART",
                        help="Serial port")
    parser.add_argument("--canid", "-c", type=int, default=11,
                        help="Target CAN ID")
    parser.add_argument("--firmware", "-f", default="/Users/bene/Dropbox/Dokumente/Promotion/PROJECTS/UC2-REST/binaries/latest/esp32_seeed_xiao_esp32s3_can_slave_motor.bin",
                        help="Firmware binary file")
    parser.add_argument("--chunk-size", type=int, default=CHUNK_SIZE,
                        help=f"Chunk size (default: {CHUNK_SIZE})")
    parser.add_argument("--high-baud", type=int, default=HIGH_BAUDRATE,
                        help=f"High baudrate for data transfer (default: {HIGH_BAUDRATE})")
    
    args = parser.parse_args()
    
    uploader = BinaryOTAUploader(
        port=args.port,
        high_baud=args.high_baud,
        chunk_size=args.chunk_size
    )
    
    success = uploader.upload_firmware(
        canid=args.canid,
        firmware_path=args.firmware
    )
    
    exit(0 if success else 1)


if __name__ == "__main__":
    main()
