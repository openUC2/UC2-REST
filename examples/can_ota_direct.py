#!/usr/bin/env python3
"""
CAN OTA Firmware Updater
========================

This module provides functionality to upload firmware to UC2 ESP32 devices
via CAN bus using the ISO-TP protocol. This enables firmware updates without
requiring WiFi connectivity on the target device.

Protocol Overview:
1. Read firmware binary file from disk
2. Calculate MD5 hash for integrity verification
3. Split firmware into chunks (default 2KB)
4. Send OTA_CAN_START command with firmware metadata
5. Send each chunk with CRC32 checksum
6. Request MD5 verification after all chunks sent
7. Send FINISH command to trigger reboot

Usage:
    from uc2rest import UC2Client
    
    client = UC2Client(serialport="/dev/ttyUSB0")
    
    # Upload firmware to motor controller (CAN ID 11)
    success = client.can_ota_direct.upload_firmware(
        can_id=11,
        firmware_path="firmware.bin",
        progress_callback=lambda sent, total: print(f"{sent}/{total} bytes")
    )

Author: UC2 Team
License: MIT
"""

import base64
import hashlib
import struct
import time
import zlib
from pathlib import Path
from typing import Callable, Optional, Dict, Any, Union
from dataclasses import dataclass
from enum import IntEnum

# Default timeout for responses (seconds)
DEFAULT_TIMEOUT = 10

# Default chunk size (must match CAN_OTA_CHUNK_SIZE in CanOtaTypes.h)
DEFAULT_CHUNK_SIZE = 2048

# Maximum firmware size (must match CAN_OTA_MAX_FIRMWARE_SIZE)
MAX_FIRMWARE_SIZE = 0x140000  # 1.25MB


class CANOtaStatus(IntEnum):
    """OTA operation status codes (must match CANOtaStatus enum in C++)"""
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
class OtaProgress:
    """Progress information for OTA upload"""
    bytes_sent: int
    bytes_total: int
    chunks_sent: int
    chunks_total: int
    elapsed_time: float
    estimated_remaining: float
    
    @property
    def percent(self) -> float:
        if self.bytes_total == 0:
            return 0.0
        return (self.bytes_sent / self.bytes_total) * 100
    
    @property
    def speed_kbps(self) -> float:
        if self.elapsed_time == 0:
            return 0.0
        return (self.bytes_sent / 1024) / self.elapsed_time


class CANOTADirect:
    """
    CAN OTA Direct Firmware Updater
    
    Enables firmware updates over CAN bus without requiring WiFi on the target device.
    Uses ISO-TP protocol for message fragmentation and reassembly.
    """
    
    def __init__(self, parent, chunk_size: int = DEFAULT_CHUNK_SIZE):
        """
        Initialize CAN OTA Direct updater.
        
        Args:
            parent: Parent UC2Client instance with post_json method
            chunk_size: Size of each firmware chunk (default 2048 bytes)
        """
        self._parent = parent
        self.chunk_size = chunk_size
        self._abort_requested = False
        
        # Callback functions
        self._progress_callback: Optional[Callable[[OtaProgress], None]] = None
        self._status_callback: Optional[Callable[[Dict], None]] = None
        
        # Register serial callback for OTA responses
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(
                self._handle_ota_response, 
                pattern="can_ota"
            )
    
    def _handle_ota_response(self, data: Dict) -> None:
        """Handle incoming OTA response messages from serial."""
        if "can_ota" in data and self._status_callback:
            self._status_callback(data["can_ota"])
    
    def register_progress_callback(self, callback: Callable[[OtaProgress], None]) -> None:
        """Register callback for progress updates."""
        self._progress_callback = callback
    
    def register_status_callback(self, callback: Callable[[Dict], None]) -> None:
        """Register callback for status updates from device."""
        self._status_callback = callback
    
    def abort(self) -> None:
        """Request abortion of current OTA upload."""
        self._abort_requested = True
    
    def _send_command(self, can_id: int, action: str, **kwargs) -> Dict:
        """
        Send OTA command via JSON over serial.
        
        Args:
            can_id: Target CAN device ID
            action: OTA action (start, data, verify, finish, abort, status)
            **kwargs: Additional command parameters
            
        Returns:
            Response dictionary from device
        """
        payload = {
            "task": "/can_ota",
            "canid": can_id,
            "action": action,
            **kwargs
        }
        
        return self._parent.post_json(
            "/can_ota",
            payload,
            getReturn=True,
            timeout=DEFAULT_TIMEOUT,
            nResponses=1
        )
    
    def _check_response(self, response: Dict) -> tuple[bool, str]:
        """
        Check if response indicates success.
        
        Returns:
            Tuple of (success: bool, error_message: str)
        """
        if response is None:
            return False, "No response received"
        
        if "error" in response:
            return False, response["error"]
        
        status = response.get("status", -1)
        if status == CANOtaStatus.OK:
            return True, ""
        
        # Map status codes to error messages
        error_messages = {
            CANOtaStatus.ERR_BUSY: "OTA already in progress",
            CANOtaStatus.ERR_INVALID_SIZE: "Invalid firmware size",
            CANOtaStatus.ERR_PARTITION: "Partition error",
            CANOtaStatus.ERR_BEGIN: "Failed to begin update",
            CANOtaStatus.ERR_WRITE: "Flash write error",
            CANOtaStatus.ERR_CRC: "CRC checksum mismatch",
            CANOtaStatus.ERR_SEQUENCE: "Chunk sequence error",
            CANOtaStatus.ERR_MD5: "MD5 verification failed",
            CANOtaStatus.ERR_END: "Failed to finalize update",
            CANOtaStatus.ERR_TIMEOUT: "Operation timeout",
            CANOtaStatus.ERR_ABORTED: "OTA was aborted",
            CANOtaStatus.ERR_MEMORY: "Memory allocation failed",
            CANOtaStatus.ERR_NOT_STARTED: "OTA not started",
        }
        
        error_msg = error_messages.get(status, f"Unknown error (status={status})")
        return False, error_msg
    
    def upload_firmware(
        self,
        can_id: int,
        firmware_path: Union[str, Path],
        progress_callback: Optional[Callable[[OtaProgress], None]] = None,
        verify: bool = True,
        auto_reboot: bool = True
    ) -> bool:
        """
        Upload firmware to CAN device via ISO-TP protocol.
        
        Args:
            can_id: Target device CAN ID (e.g., 11 for Motor X)
            firmware_path: Path to firmware binary file (.bin)
            progress_callback: Optional callback for progress updates
            verify: Whether to verify MD5 after upload (default True)
            auto_reboot: Whether to automatically reboot device (default True)
            
        Returns:
            True if upload successful, False otherwise
            
        Raises:
            FileNotFoundError: If firmware file doesn't exist
            ValueError: If firmware size exceeds maximum
        """
        self._abort_requested = False
        firmware_path = Path(firmware_path)
        
        # Read and validate firmware
        if not firmware_path.exists():
            raise FileNotFoundError(f"Firmware file not found: {firmware_path}")
        
        firmware_data = firmware_path.read_bytes()
        firmware_size = len(firmware_data)
        
        if firmware_size > MAX_FIRMWARE_SIZE:
            raise ValueError(
                f"Firmware size ({firmware_size} bytes) exceeds maximum "
                f"({MAX_FIRMWARE_SIZE} bytes)"
            )
        
        if firmware_size == 0:
            raise ValueError("Firmware file is empty")
        
        # Calculate metadata
        md5_hash = hashlib.md5(firmware_data).hexdigest()
        num_chunks = (firmware_size + self.chunk_size - 1) // self.chunk_size
        
        print(f"[CAN OTA] Firmware: {firmware_path.name}")
        print(f"[CAN OTA] Size: {firmware_size} bytes ({num_chunks} chunks)")
        print(f"[CAN OTA] MD5: {md5_hash}")
        print(f"[CAN OTA] Target CAN ID: {can_id}")
        
        start_time = time.time()
        
        try:
            # Step 1: Send START command
            print("[CAN OTA] Sending START command...")
            response = self._send_command(
                can_id,
                "start",
                firmware_size=firmware_size,
                chunk_size=self.chunk_size,
                total_chunks=num_chunks,
                md5=md5_hash
            )
            
            success, error = self._check_response(response)
            if not success:
                print(f"[CAN OTA] START failed: {error}")
                return False
            
            print("[CAN OTA] Device ready, starting upload...")
            
            # Step 2: Send chunks
            bytes_sent = 0
            for chunk_idx in range(num_chunks):
                if self._abort_requested:
                    print("[CAN OTA] Abort requested")
                    self._send_command(can_id, "abort")
                    return False
                
                # Extract chunk data
                chunk_start = chunk_idx * self.chunk_size
                chunk_end = min(chunk_start + self.chunk_size, firmware_size)
                chunk_data = firmware_data[chunk_start:chunk_end]
                actual_chunk_size = len(chunk_data)
                
                # Calculate CRC32
                chunk_crc = zlib.crc32(chunk_data) & 0xFFFFFFFF
                
                # Encode chunk as base64 for JSON transport
                chunk_b64 = base64.b64encode(chunk_data).decode('ascii')
                
                # Send chunk with retry logic
                max_retries = 3
                for retry in range(max_retries):
                    response = self._send_command(
                        can_id,
                        "data",
                        chunk_index=chunk_idx,
                        chunk_size=actual_chunk_size,
                        chunk_crc=chunk_crc,
                        data=chunk_b64
                    )
                    
                    success, error = self._check_response(response)
                    if success:
                        break
                    
                    if retry < max_retries - 1:
                        print(f"[CAN OTA] Chunk {chunk_idx} failed ({error}), retrying...")
                        time.sleep(0.1)
                    else:
                        print(f"[CAN OTA] Chunk {chunk_idx} failed after {max_retries} retries")
                        self._send_command(can_id, "abort")
                        return False
                
                bytes_sent = chunk_end
                elapsed = time.time() - start_time
                
                # Calculate progress
                progress = OtaProgress(
                    bytes_sent=bytes_sent,
                    bytes_total=firmware_size,
                    chunks_sent=chunk_idx + 1,
                    chunks_total=num_chunks,
                    elapsed_time=elapsed,
                    estimated_remaining=(elapsed / bytes_sent * (firmware_size - bytes_sent)) if bytes_sent > 0 else 0
                )
                
                # Call progress callback
                if progress_callback:
                    progress_callback(progress)
                elif self._progress_callback:
                    self._progress_callback(progress)
                
                # Print progress every 10%
                if (chunk_idx + 1) % max(1, num_chunks // 10) == 0:
                    print(f"[CAN OTA] Progress: {progress.percent:.1f}% ({progress.speed_kbps:.1f} KB/s)")
            
            print("[CAN OTA] All chunks sent")
            
            # Step 3: Verify MD5
            if verify:
                print("[CAN OTA] Requesting MD5 verification...")
                response = self._send_command(
                    can_id,
                    "verify",
                    md5=md5_hash
                )
                
                success, error = self._check_response(response)
                if not success:
                    print(f"[CAN OTA] Verification failed: {error}")
                    self._send_command(can_id, "abort")
                    return False
                
                print("[CAN OTA] MD5 verification passed")
            
            # Step 4: Finish and reboot
            if auto_reboot:
                print("[CAN OTA] Sending FINISH command (device will reboot)...")
                response = self._send_command(can_id, "finish")
                
                success, error = self._check_response(response)
                if not success:
                    print(f"[CAN OTA] FINISH failed: {error}")
                    return False
            
            elapsed = time.time() - start_time
            speed = firmware_size / 1024 / elapsed if elapsed > 0 else 0
            
            print(f"[CAN OTA] Upload complete!")
            print(f"[CAN OTA] Time: {elapsed:.1f}s, Speed: {speed:.1f} KB/s")
            
            return True
            
        except Exception as e:
            print(f"[CAN OTA] Error: {e}")
            try:
                self._send_command(can_id, "abort")
            except:
                pass
            return False
    
    def get_status(self, can_id: int) -> Optional[Dict]:
        """
        Query OTA status from device.
        
        Args:
            can_id: Target device CAN ID
            
        Returns:
            Status dictionary or None if query failed
        """
        response = self._send_command(can_id, "status")
        if response and "status" in response:
            return response
        return None
    
    def abort_update(self, can_id: int) -> bool:
        """
        Send abort command to cancel ongoing OTA.
        
        Args:
            can_id: Target device CAN ID
            
        Returns:
            True if abort acknowledged
        """
        response = self._send_command(can_id, "abort")
        success, _ = self._check_response(response)
        return success


# Convenience functions for common CAN IDs
class CANOTADirectMotor(CANOTADirect):
    """CAN OTA updater with motor-specific helpers"""
    
    # Standard motor CAN IDs
    MOTOR_A = 10
    MOTOR_X = 11
    MOTOR_Y = 12
    MOTOR_Z = 13
    MOTOR_B = 14
    MOTOR_C = 15
    
    def upload_to_motor(
        self,
        axis: Union[str, int],
        firmware_path: Union[str, Path],
        **kwargs
    ) -> bool:
        """
        Upload firmware to motor controller.
        
        Args:
            axis: Motor axis ("X", "Y", "Z") or CAN ID (11, 12, 13)
            firmware_path: Path to firmware binary
            **kwargs: Additional arguments passed to upload_firmware
        """
        if isinstance(axis, str):
            axis_map = {
                "A": self.MOTOR_A,
                "X": self.MOTOR_X,
                "Y": self.MOTOR_Y,
                "Z": self.MOTOR_Z,
                "B": self.MOTOR_B,
                "C": self.MOTOR_C,
            }
            can_id = axis_map.get(axis.upper())
            if can_id is None:
                raise ValueError(f"Unknown motor axis: {axis}")
        else:
            can_id = axis
        
        return self.upload_firmware(can_id, firmware_path, **kwargs)


# Standalone usage
if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Upload firmware to UC2 device via CAN bus"
    )
    parser.add_argument(
        "--port", "-p",
        required=True,
        help="Serial port (e.g., /dev/ttyUSB0, COM3)"
    )
    parser.add_argument(
        "--canid", "-c",
        type=int,
        required=True,
        help="Target CAN ID (e.g., 11 for Motor X)"
    )
    parser.add_argument(
        "--firmware", "-f",
        required=True,
        help="Path to firmware binary file"
    )
    parser.add_argument(
        "--no-verify",
        action="store_true",
        help="Skip MD5 verification"
    )
    parser.add_argument(
        "--no-reboot",
        action="store_true",
        help="Don't reboot device after upload"
    )
    
    args = parser.parse_args()
    
    # Try to import UC2Client
    try:
        from uc2rest import UC2Client
        
        print(f"Connecting to {args.port}...")
        client = UC2Client(serialport=args.port)
        
        # Create OTA updater
        ota = CANOTADirect(client)
        
        # Upload firmware
        success = ota.upload_firmware(
            can_id=args.canid,
            firmware_path=args.firmware,
            verify=not args.no_verify,
            auto_reboot=not args.no_reboot
        )
        
        exit(0 if success else 1)
        
    except ImportError:
        print("UC2Client not available. Please install uc2rest package.")
        print("pip install -e /path/to/UC2-REST")
        exit(1)
