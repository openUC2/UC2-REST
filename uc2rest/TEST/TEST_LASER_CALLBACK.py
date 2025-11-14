#!/usr/bin/env python3
"""
Example usage of the updated laser callback functionality.

This script demonstrates how to:
1. Use the updated laser callback mechanism
2. Register custom callbacks for laser status updates
3. Monitor laser value changes through callbacks

Copyright 2025 Benedict Diederich, released under LGPL 3.0 or later
"""

import uc2rest
import time

# Example configuration
SERIALPORT = "/dev/cu.SLAB_USBtoUART"  # Adjust to your serial port

def main():
    # Connect to UC2 device via serial
    print("Connecting to UC2 device...")
    ESP32 = uc2rest.UC2Client(serialport=SERIALPORT, baudrate=115200, DEBUG=True)
    
    if not ESP32.is_connected:
        print("Failed to connect to UC2 device!")
        return
    
    print("Connected successfully!")
    
    # Define callback function for laser status updates
    def laser_callback(laser_values):
        """
        Callback function to handle laser status updates.
        
        :param laser_values: Array containing current laser values [laser0, laser1, laser2, laser3]
        """
        laser_names = ["Laser 0", "Laser 1 (R)", "Laser 2 (G)", "Laser 3 (B)"]
        print(f"\n=== Laser Status Update ===")
        for i, value in enumerate(laser_values):
            if value > 0:
                print(f"{laser_names[i]}: {value} (ON)")
            else:
                print(f"{laser_names[i]}: {value} (OFF)")
        print("==========================\n")
    
    # Register the callback for laser status (key 0)
    ESP32.laser.register_callback(0, laser_callback)
    
    # Example 1: Set laser 1 (Red) to medium intensity
    print("Example 1: Setting Laser 1 (Red) to 512...")
    response = ESP32.laser.set_laser(channel=1, value=512, is_blocking=False)
    print(f"Command sent: {response}")
    
    # Wait for response
    time.sleep(2)
    
    # Example 2: Set laser 2 (Green) to full intensity
    print("\nExample 2: Setting Laser 2 (Green) to 1023...")
    response = ESP32.laser.set_laser(channel=2, value=1023, is_blocking=False)
    print(f"Command sent: {response}")
    
    # Wait for response
    time.sleep(2)
    
    # Example 3: Turn off all lasers
    print("\nExample 3: Turning off all lasers...")
    for channel in range(4):
        response = ESP32.laser.set_laser(channel=channel, value=0, is_blocking=False)
        print(f"Laser {channel} off: {response}")
        time.sleep(0.5)
    
    # Example 4: Multiple laser operations
    print("\nExample 4: Setting multiple lasers...")
    laser_settings = [
        {"channel": 1, "value": 256},  # Red at 25%
        {"channel": 2, "value": 512},  # Green at 50%
        {"channel": 3, "value": 768},  # Blue at 75%
    ]
    
    for setting in laser_settings:
        response = ESP32.laser.set_laser(
            channel=setting["channel"], 
            value=setting["value"], 
            is_blocking=False
        )
        print(f"Laser {setting['channel']} set to {setting['value']}: {response}")
        time.sleep(1)
    
    # Keep the script running to receive callbacks
    print("\nListening for laser status updates... (Press Ctrl+C to exit)")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nExiting...")

def example_laser_with_despeckle():
    """
    Example using laser with despeckle functionality.
    """
    ESP32 = uc2rest.UC2Client(serialport=SERIALPORT, baudrate=115200)
    
    if not ESP32.is_connected:
        print("Failed to connect!")
        return
    
    # Register callback
    def despeckle_callback(laser_values):
        print(f"Despeckle laser callback: {laser_values}")
    
    ESP32.laser.register_callback(0, despeckle_callback)
    
    # Set laser with despeckle
    print("Setting laser with despeckle...")
    ESP32.laser.set_laser(
        channel=1,
        value=800,
        despeckleAmplitude=0.1,  # 10% amplitude
        despecklePeriod=50,      # 50ms period
        is_blocking=False
    )
    
    # Wait for updates
    time.sleep(5)

def example_color_lasers():
    """
    Example using color channel names instead of numbers.
    """
    ESP32 = uc2rest.UC2Client(serialport=SERIALPORT, baudrate=115200)
    
    if not ESP32.is_connected:
        print("Failed to connect!")
        return
    
    # Register callback
    def color_callback(laser_values):
        colors = ["White", "Red", "Green", "Blue"]
        active_colors = [colors[i] for i, val in enumerate(laser_values) if val > 0]
        if active_colors:
            print(f"Active colors: {', '.join(active_colors)}")
        else:
            print("All lasers off")
    
    ESP32.laser.register_callback(0, color_callback)
    
    # Test color channels
    print("Testing color channels...")
    colors = ["R", "G", "B"]  # Red, Green, Blue
    
    for color in colors:
        print(f"Setting {color} channel...")
        ESP32.laser.set_laser(channel=color, value=500, is_blocking=False)
        time.sleep(2)
        
        # Turn off
        ESP32.laser.set_laser(channel=color, value=0, is_blocking=False)
        time.sleep(1)

if __name__ == "__main__":
    choice = input("Choose example:\n1. Main example\n2. Despeckle example\n3. Color channels\nEnter choice (1, 2, or 3): ")
    
    if choice == "1":
        main()
    elif choice == "2":
        example_laser_with_despeckle()
    elif choice == "3":
        example_color_lasers()
    else:
        print("Invalid choice!")