#!/usr/bin/env python3
"""
Example usage of the CAN OTA update functionality.

This script demonstrates how to:
1. Start OTA update on various CAN devices
2. Register callbacks to handle OTA responses
3. Get the appropriate upload commands for firmware updates

Copyright 2025 Benedict Diederich, released under LGPL 3.0 or later
"""

import uc2rest
import time

# Example configuration
SERIALPORT = "/dev/cu.SLAB_USBtoUART"  # Adjust to your serial port
WIFI_SSID = "Blynk"
WIFI_PASSWORD = "12345678"

def main():
    # Connect to UC2 device via serial
    print("Connecting to UC2 device...")
    ESP32 = uc2rest.UC2Client(serialport=SERIALPORT, baudrate=115200, DEBUG=True)
    
    if not ESP32.is_connected:
        print("Failed to connect to UC2 device!")
        return
    
    print("Connected successfully!")
    
    # Define callback function for OTA status updates
    def ota_callback(ota_response):
        """
        Callback function to handle OTA status updates.
        
        :param ota_response: Dictionary containing OTA status information
        """
        can_id = ota_response["canId"]
        status = ota_response["status"]
        status_msg = ota_response["statusMsg"]
        ip_address = ota_response["ip"]
        hostname = ota_response["hostname"]
        
        print(f"\n=== OTA Status Update ===")
        print(f"CAN ID: {can_id}")
        print(f"Status: {status} ({status_msg})")
        
        if ota_response["success"]:
            print(f"✅ Device ready for OTA!")
            print(f"IP Address: {ip_address}")
            print(f"Hostname: {hostname}")
            print(f"Upload command: {ESP32.canota.get_platformio_upload_command(can_id)}")
        else:
            print(f"❌ OTA setup failed: {status_msg}")
        print("========================\n")
    
    # Register the callback for all OTA events (key 0)
    ESP32.canota.register_callback(0, ota_callback)
    
    # Example 1: Start OTA update on Motor X (CAN ID 11)
    print("Starting OTA update on Motor X (CAN ID 11)...")
    response = ESP32.canota.start_laser_ota(laser_id=0, ssid=WIFI_SSID, password=WIFI_PASSWORD, timeout=300000, is_blocking=False)
    # response = ESP32.canota.start_motor_ota("X", WIFI_SSID, WIFI_PASSWORD, timeout=300000) # tODO: we should have a generic interface (e.g. by ID instead of by motor, etc. )
    print(f"Command sent: {response}")
    
    # Wait for response
    print("Waiting for OTA response...")
    time.sleep(10) # TODO: We should be wait for a positible callback 
    
    # Example 2: Start OTA update on Laser (CAN ID 20)
    print("\nStarting OTA update on Laser (CAN ID 20)...")
    response = ESP32.canota.start_laser_ota(0, WIFI_SSID, WIFI_PASSWORD)
    print(f"Command sent: {response}")
    
    # Wait for response
    time.sleep(5)
    
    # Example 3: Start OTA update on LED Controller (CAN ID 30)
    print("\nStarting OTA update on LED Controller (CAN ID 30)...")
    response = ESP32.canota.start_led_ota(WIFI_SSID, WIFI_PASSWORD)
    print(f"Command sent: {response}")
    
    # Keep the script running to receive callbacks
    print("\nListening for OTA responses... (Press Ctrl+C to exit)")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nExiting...")

def example_direct_can_id():
    """
    Example using direct CAN IDs instead of convenience methods.
    """
    ESP32 = uc2rest.UC2Client(serialport=SERIALPORT, baudrate=115200)
    
    if not ESP32.is_connected:
        print("Failed to connect!")
        return
    
    # Register callback
    def simple_callback(ota_response):
        if ota_response["success"]:
            print(f"Device {ota_response['canId']} ready at {ota_response['ip']}")
        else:
            print(f"OTA failed for device {ota_response['canId']}")
    
    ESP32.canota.register_callback(0, simple_callback)
    
    # Send OTA command to specific CAN devices
    devices = [
        {"canid": 11, "name": "Motor X"},
        {"canid": 12, "name": "Motor Y"}, 
        {"canid": 13, "name": "Motor Z"},
        {"canid": 20, "name": "Laser"},
        {"canid": 30, "name": "LED Controller"}
    ]
    
    for device in devices:
        print(f"Starting OTA for {device['name']} (CAN ID {device['canid']})...")
        ESP32.canota.start_ota_update(
            can_id=device["canid"],
            ssid=WIFI_SSID,
            password=WIFI_PASSWORD,
            timeout=600000  # 10 minutes
        )
        time.sleep(2)  # Small delay between commands
    
    # Wait for responses
    time.sleep(30)

def example_platformio_commands():
    """
    Example showing how to get PlatformIO upload commands.
    """
    ESP32 = uc2rest.UC2Client(serialport=SERIALPORT, baudrate=115200)
    
    print("PlatformIO upload commands for different devices:")
    print("=" * 50)
    
    devices = [11, 12, 13, 20, 30]  # Motor X, Y, Z, Laser, LED
    
    for can_id in devices:
        hostname = ESP32.canota.get_ota_hostname(can_id)
        upload_cmd = ESP32.canota.get_platformio_upload_command(can_id)
        print(f"CAN ID {can_id:2d}: {hostname}")
        print(f"         {upload_cmd}")
        print()

if __name__ == "__main__":
    choice = 1
    if choice == 1:
        main()
    elif choice == "2":
        example_direct_can_id()
    elif choice == "3":
        example_platformio_commands()
    else:
        print("Invalid choice!")