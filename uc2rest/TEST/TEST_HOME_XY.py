#!/usr/bin/env python3
"""
Example usage of the home_xy functionality.

This script demonstrates how to:
1. Home multiple motors simultaneously
2. Use different parameters for each motor
3. Register callbacks to monitor homing status

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
    
    # Define callback function for homing status updates
    def home_callback(is_homed):
        """
        Callback function to handle homing status updates.
        
        :param is_homed: Array indicating which motors are homed [A, X, Y, Z]
        """
        motor_names = ["A", "X", "Y", "Z"]
        print(f"\n=== Homing Status Update ===")
        for i, homed in enumerate(is_homed):
            status = "✅ HOMED" if homed else "❌ NOT HOMED"
            print(f"Motor {motor_names[i]}: {status}")
        print("===========================\n")
    
    # Register the callback for homing status (key 0)
    ESP32.home.register_callback(0, home_callback)
    
    # Example 1: Home X and Y axes with the example parameters from the user
    print("Example 1: Homing X and Y axes with specific parameters...")
    response = ESP32.home.home_xy(
        axes=["X", "Y"],  # Home X (stepperid=1) and Y (stepperid=2)
        speeds=[15000, 15000],  # Speed for both motors
        directions=[1, -1],  # X moves in positive direction, Y in negative
        timeouts=[200, 400],  # X timeout 200ms, Y timeout 400ms
        endstoppolarities=[1, 1],  # Both use same endstop polarity
        isBlocking=False  # Don't wait for completion
    )
    print(f"Command sent: {response}")
    
    # Wait for homing to complete
    print("Waiting for homing to complete...")
    time.sleep(5)
    
    # Example 2: Home all axes with same parameters
    print("\nExample 2: Homing all axes with same parameters...")
    response = ESP32.home.home_xy(
        axes=["X", "Y", "Z"],  # Home X, Y, and Z
        speeds=10000,  # Same speed for all
        directions=-1,  # All move in negative direction
        timeouts=5000,  # 5 second timeout for all
        isBlocking=True  # Wait for completion
    )
    print(f"Homing completed: {response}")
    
    # Example 3: Home with stepper IDs directly
    print("\nExample 3: Using stepper IDs directly...")
    response = ESP32.home.home_xy(
        axes=[1, 2],  # Stepper IDs 1 and 2 (X and Y)
        speeds=[12000, 12000],
        directions=[1, 1],
        timeouts=[300, 300],
        isBlocking=False
    )
    print(f"Command sent: {response}")
    
    # Keep the script running to receive callbacks
    print("\nListening for homing status updates... (Press Ctrl+C to exit)")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nExiting...")

def example_individual_vs_simultaneous():
    """
    Compare individual homing vs simultaneous homing.
    """
    ESP32 = uc2rest.UC2Client(serialport=SERIALPORT, baudrate=115200)
    
    if not ESP32.is_connected:
        print("Failed to connect!")
        return
    
    print("=== Individual Homing ===")
    start_time = time.time()
    
    # Home X and Y individually
    ESP32.home.home_x(speed=15000, direction=1, timeout=200, isBlocking=True)
    ESP32.home.home_y(speed=15000, direction=-1, timeout=400, isBlocking=True)
    
    individual_time = time.time() - start_time
    print(f"Individual homing took: {individual_time:.2f} seconds")
    
    print("\n=== Simultaneous Homing ===")
    start_time = time.time()
    
    # Home X and Y simultaneously
    ESP32.home.home_xy(
        axes=["X", "Y"],
        speeds=[15000, 15000],
        directions=[1, -1],
        timeouts=[200, 400],
        isBlocking=True
    )
    
    simultaneous_time = time.time() - start_time
    print(f"Simultaneous homing took: {simultaneous_time:.2f} seconds")
    print(f"Time saved: {individual_time - simultaneous_time:.2f} seconds")

if __name__ == "__main__":
    choice = input("Choose example:\n1. Main example\n2. Individual vs Simultaneous\nEnter choice (1 or 2): ")
    
    if choice == "1":
        main()
    elif choice == "2":
        example_individual_vs_simultaneous()
    else:
        print("Invalid choice!")