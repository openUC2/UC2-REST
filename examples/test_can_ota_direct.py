#!/usr/bin/env python3
"""
Standalone CAN OTA Test Script
===============================
Direct serial communication for CAN OTA firmware upload
without UC2Client dependency.
"""

import serial
import time
import json
import base64
import hashlib
import struct
import zlib
from pathlib import Path
from typing import Dict, Any, Optional

# Constants
DEFAULT_CHUNK_SIZE = 2048
DEFAULT_TIMEOUT = 5.0
MAX_RETRIES = 3

def calculate_crc32(data: bytes) -> int:
    """Calculate CRC32 for chunk data."""
    return zlib.crc32(data) & 0xFFFFFFFF

def calculate_md5(data: bytes) -> str:
    """Calculate MD5 hash of firmware."""
    return hashlib.md5(data).hexdigest()

def is_response_success(response: Dict) -> bool:
    """Check if response indicates success."""
    if not isinstance(response, dict):
        return False
    
    # Check for success field
    success = response.get('success', False)
    if success is True or success == 'true' or success == 1 or success == '1':
        return True
    
    # Check for error field - if present, it's a failure
    if 'error' in response:
        return False
    
    return False

def get_response_error(response: Dict) -> str:
    """Extract error message from response."""
    if not isinstance(response, dict):
        return str(response)
    
    return response.get('error', 'Unknown error')

def send_can_ota_command(ser: serial.Serial, canid: int, action: str, 
                         timeout: float = DEFAULT_TIMEOUT, **kwargs) -> Optional[Dict]:
    """
    Send CAN OTA command and wait for response.
    
    Args:
        ser: Serial port object
        canid: Target CAN ID
        action: Command action (start, data, end, verify, abort, status)
        timeout: Response timeout in seconds
        **kwargs: Additional command parameters
    
    Returns:
        Response dictionary or None if timeout
    """
    # Construct command
    command = {
        "task": "/can_ota",
        "canid": canid,
        "action": action,
        **kwargs
    }
    
    
    # Serialize to JSON
    json_str = json.dumps(command, separators=(',', ':'))
    
    # Clear input buffer
    ser.reset_input_buffer()
    
    # Send command
    print(f"TX: {json_str}")
    ser.write((json_str + '\n').encode('utf-8'))
    ser.flush()
    
    # Wait for response
    start_time = time.time()
    response_lines = []
    
    while (time.time() - start_time) < timeout:
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"RX: {line}")
                    response_lines.append(line)
                    
                    # Try to parse as JSON
                    if line.startswith('{') and line.endswith('}'):
                        try:
                            response = json.loads(line)
                            return response
                        except json.JSONDecodeError:
                            continue
            except UnicodeDecodeError:
                continue
            except Exception as e:
                print(f"Error reading response: {e}")
                continue
        
        time.sleep(0.01)  # Small delay to avoid busy waiting
    
    print(f"Timeout waiting for response to '{action}' command")
    return None

def upload_firmware_direct(
    port: str,
    baudrate: int,
    canid: int,
    firmware_path: str,
    chunk_size: int = DEFAULT_CHUNK_SIZE
) -> bool:
    """
    Upload firmware via CAN OTA using direct serial communication.
    
    Args:
        port: Serial port path (e.g., /dev/cu.SLAB_USBtoUART)
        baudrate: Serial baudrate (typically 921600)
        canid: Target CAN device ID
        firmware_path: Path to firmware binary file
        chunk_size: Size of each chunk in bytes
    
    Returns:
        True if upload successful, False otherwise
    """
    firmware_path = Path(firmware_path)
    
    # Validate firmware file
    if not firmware_path.exists():
        print(f"Error: Firmware file not found: {firmware_path}")
        return False
    
    # Read firmware
    firmware_data = firmware_path.read_bytes()
    firmware_size = len(firmware_data)
    
    if firmware_size == 0:
        print("Error: Firmware file is empty")
        return False
    
    # Calculate metadata
    md5_hash = calculate_md5(firmware_data)
    num_chunks = (firmware_size + chunk_size - 1) // chunk_size
    
    print("=" * 60)
    print("CAN OTA Direct Firmware Upload")
    print("=" * 60)
    print(f"Serial Port: {port}")
    print(f"Target CAN ID: {canid}")
    print(f"Firmware: {firmware_path}")
    print()
    print("Firmware Info:")
    print(f"  Size: {firmware_size:,} bytes")
    print(f"  Chunks: {num_chunks}")
    print(f"  MD5: {md5_hash}")
    print("=" * 60)
    print()
    
    try:
        # Open serial port
        print(f"Opening serial port {port}...")
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=1.0,
            write_timeout=1.0
        )
        
        time.sleep(0.5)  # Allow port to stabilize
        print()
        
        # Step 1: Send START command
        print("[1/4] Sending START command...")
        response = send_can_ota_command(
            ser, canid, "start",
            firmware_size=firmware_size,
            chunk_size=chunk_size,
            total_chunks=num_chunks,
            md5=md5_hash,
            timeout=10.0
        )
        
        if not response or not is_response_success(response):
            error = get_response_error(response) if response else "No response"
            print(f"  START command failed: {error}")
            ser.close()
            return False
        
        print("  Device ready!")
        print()
        
        # Step 2: Upload chunks
        print(f"[2/4] Uploading {num_chunks} chunks...")
        start_time = time.time()
        failed_chunks = 0
        
        for chunk_idx in range(num_chunks):
            # Extract chunk
            chunk_start = chunk_idx * chunk_size
            chunk_end = min(chunk_start + chunk_size, firmware_size)
            chunk_data = firmware_data[chunk_start:chunk_end]
            actual_chunk_size = len(chunk_data)
            
            # Calculate CRC32
            chunk_crc = calculate_crc32(chunk_data)
            
            # Encode as base64
            chunk_b64 = base64.b64encode(chunk_data).decode('ascii')
            
            # Send chunk with retry
            retry_count = 0
            success = False
            
            while retry_count < MAX_RETRIES and not success:
                response = send_can_ota_command(
                    ser, canid, "data",
                    chunk_index=chunk_idx,
                    chunk_size=actual_chunk_size,
                    chunk_crc=chunk_crc,
                    data=chunk_b64,
                    timeout=DEFAULT_TIMEOUT
                )
                
                if response and is_response_success(response):
                    success = True
                else:
                    retry_count += 1
                    if retry_count < MAX_RETRIES:
                        error = get_response_error(response) if response else "No response"
                        print(f"  Chunk {chunk_idx} failed, retrying ({retry_count}/{MAX_RETRIES})...")
                        time.sleep(0.2)
            
            if not success:
                error = get_response_error(response) if response else "No response"
                print(f"\nOTA Error: Chunk {chunk_idx} failed after {MAX_RETRIES} retries: {error}")
                
                # Send abort
                send_can_ota_command(ser, canid, "abort", timeout=2.0)
                ser.close()
                return False
            
            # Print progress
            if (chunk_idx + 1) % 10 == 0 or chunk_idx == num_chunks - 1:
                elapsed = time.time() - start_time
                percent = ((chunk_idx + 1) / num_chunks) * 100
                bytes_sent = chunk_end
                speed_kbps = (bytes_sent / 1024) / elapsed if elapsed > 0 else 0
                eta = (elapsed / (chunk_idx + 1) * (num_chunks - chunk_idx - 1)) if chunk_idx > 0 else 0
                
                print(f"  Progress: {chunk_idx + 1}/{num_chunks} ({percent:.1f}%) | "
                      f"Speed: {speed_kbps:.1f} KB/s | ETA: {eta:.0f}s")
        
        elapsed = time.time() - start_time
        avg_speed = (firmware_size / 1024) / elapsed if elapsed > 0 else 0
        
        print(f"  Upload complete: {elapsed:.1f}s, {avg_speed:.1f} KB/s")
        print()
        
        # Step 3: Send END command
        print("[3/4] Finalizing OTA...")
        response = send_can_ota_command(
            ser, canid, "end",
            timeout=10.0
        )
        
        if not response or not is_response_success(response):
            error = get_response_error(response) if response else "No response"
            print(f"  END command failed: {error}")
            send_can_ota_command(ser, canid, "abort", timeout=2.0)
            ser.close()
            return False
        
        print("  OTA finalized")
        print()
        
        # Step 4: Verify MD5
        print("[4/4] Verifying firmware...")
        response = send_can_ota_command(
            ser, canid, "verify",
            md5=md5_hash,
            timeout=15.0
        )
        
        if not response or not is_response_success(response):
            error = get_response_error(response) if response else "No response"
            print(f"  Verification failed: {error}")
            ser.close()
            return False
        
        print("  Verification successful!")
        print()
        
        print("=" * 60)
        print("OTA UPDATE COMPLETED SUCCESSFULLY!")
        print("=" * 60)
        print(f"Total time: {elapsed:.1f}s")
        print(f"Average speed: {avg_speed:.1f} KB/s")
        print()
        print("Device will reboot with new firmware...")
        print()
        
        ser.close()
        return True
        
    except serial.SerialException as e:
        print(f"\nSerial port error: {e}")
        return False
    except KeyboardInterrupt:
        print("\n\nUpload cancelled by user")
        try:
            send_can_ota_command(ser, canid, "abort", timeout=2.0)
            ser.close()
        except:
            pass
        return False
    except Exception as e:
        print(f"\nUnexpected error: {e}")
        import traceback
        traceback.print_exc()
        try:
            send_can_ota_command(ser, canid, "abort", timeout=2.0)
            ser.close()
        except:
            pass
        return False


if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(
        description="CAN OTA Firmware Upload Test Tool"
    )
    parser.add_argument(
        "--port", "-p",
        default="/dev/cu.SLAB_USBtoUART",
        help="Serial port (default: /dev/cu.SLAB_USBtoUART)"
    )
    parser.add_argument(
        "--baudrate", "-b",
        type=int,
        default=921600,
        help="Serial baudrate (default: 921600)"
    )
    parser.add_argument(
        "--canid", "-c",
        type=int,
        default=11,
        help="Target CAN ID (default: 11)"
    )
    parser.add_argument(
        "--firmware", "-f",
        default="/Users/bene/Dropbox/Dokumente/Promotion/PROJECTS/UC2-REST/binaries/latest/esp32_seeed_xiao_esp32s3_can_slave_motor.bin",
        help="Firmware binary file path"
    )
    parser.add_argument(
        "--chunk-size",
        type=int,
        default=2048,
        help="Chunk size in bytes (default: 2048)"
    )
    
    args = parser.parse_args()
    
    success = upload_firmware_direct(
        port=args.port,
        baudrate=args.baudrate,
        canid=args.canid,
        firmware_path=args.firmware,
        chunk_size=args.chunk_size
    )
    
    exit(0 if success else 1)
