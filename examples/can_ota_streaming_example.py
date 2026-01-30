#!/usr/bin/env python3
"""
Example: CAN OTA Streaming Upload

This example demonstrates how to upload firmware to a CAN slave device
using the USB-based streaming protocol (no WiFi required).

Prerequisites:
- UC2 Master device connected via USB
- CAN slave device connected to CAN bus
- Firmware binary file (.bin)

Usage:
    python can_ota_streaming_example.py --port /dev/cu.SLAB_USBtoUART --canid 11 --firmware /path/to/firmware.bin
"""

import argparse
import sys
import time

# Add parent directory to path for imports
sys.path.insert(0, '..')

try:
    from uc2rest import UC2Client
except ImportError:
    print("ERROR: Could not import UC2Client. Make sure uc2rest is installed.")
    print("       Run: pip install -e . from the UC2-REST directory")
    sys.exit(1)


def progress_callback(page, total_pages, bytes_sent, speed_kbps):
    """Called for each page uploaded."""
    progress = page / total_pages * 100
    bar_len = 40
    filled = int(bar_len * page / total_pages)
    bar = '█' * filled + '░' * (bar_len - filled)
    print(f"\r  [{bar}] {progress:5.1f}% - Page {page}/{total_pages} - {speed_kbps:.1f} KB/s", end='', flush=True)


def status_callback(message, success):
    """Called for status updates."""
    icon = "✓" if success else "✗"
    print(f"\n[{icon}] {message}")


def main():

    port = "/dev/cu.SLAB_USBtoUART"
    canid = 11
    firmware = "/Users/bene/Dropbox/Dokumente/Promotion/PROJECTS/UC2-REST/binaries/latest/esp32_seeed_xiao_esp32s3_can_slave_motor.bin"
    baud = 921600
    
    print("=" * 70)
    print("CAN OTA STREAMING UPLOAD")
    print("=" * 70)
    print(f"Port:     {port}")
    print(f"CAN ID:   {canid}")
    print(f"Firmware: {firmware}")
    print(f"Baud:     {baud}")
    print("=" * 70)
    
    # Create UC2Client instance
    print("\n[1] Connecting to UC2 Master...")
    try:
        esp = UC2Client(serialport=port, baudrate=baud)
        if not esp.serial.is_connected:
            print("ERROR: Could not connect to UC2 Master")
            sys.exit(1)
        print(f"    Connected to {esp.serial.serialport}")
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)
    
    # Start streaming upload
    print("\n[2] Starting CAN OTA streaming upload...")
    
    start_time = time.time()
    
    success = esp.canota.start_can_streaming_ota_blocking(
        can_id=canid,
        firmware_path=firmware,
        progress_callback=progress_callback,
        status_callback=status_callback,
        baud=baud
    )
    
    elapsed = time.time() - start_time
    
    print("\n" + "=" * 70)
    if success:
        print(f"UPLOAD COMPLETE! Total time: {elapsed:.1f} seconds")
    else:
        print(f"UPLOAD FAILED after {elapsed:.1f} seconds")
    print("=" * 70)
    
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
