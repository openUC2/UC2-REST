"""
Camera trigger callback module for UC2-REST.

This module handles camera trigger signals from the firmware ({"cam":1})
to enable software triggering based on hardware events during stage scanning.
"""

import numpy as np
import time
import json


gTIMEOUT = 1  # seconds to wait for a response from the ESP32


class CameraTrigger(object):
    """
    This class parses incoming camera trigger signals from the ESP32 firmware.
    
    When the firmware sends {"cam":1}, this module triggers registered callbacks
    which can be used for software-triggered image acquisition during stage scanning.
    
    Example usage:
        import uc2rest
        
        ESP32 = uc2rest.UC2Client(serialport=port, baudrate=500000)
        
        # Register callback for camera trigger
        def my_camera_callback(data):
            print(f"Camera trigger received: {data}")
            # Trigger image acquisition here
            
        ESP32.camera_trigger.register_callback(0, my_camera_callback)
    """
    
    def __init__(self, parent=None, nCallbacks=10):
        """
        Initialize camera trigger handler.
        
        Args:
            parent: Parent UC2Client instance
            nCallbacks: Maximum number of callback functions to register
        """
        self._parent = parent
        self.nCallbacks = nCallbacks
        
        # Track trigger count for diagnostics
        self._trigger_count = 0
        self._last_trigger_time = None
        
        # Initialize callback functions
        self._callbackPerKey = {}
        self.init_callback_functions(self.nCallbacks)
        
        # Register callback for camera trigger on serial loop
        if hasattr(self._parent, "serial"):
            self._parent.serial.register_callback(self._callback_camera_trigger, pattern="cam")

    def _callback_camera_trigger(self, data):
        """
        Parse camera trigger message from firmware.
        
        Expected JSON format:
        {
            "cam": 1  # Trigger signal
        }
        
        or with additional data:
        {
            "cam": {
                "trigger": 1,
                "frame_id": 123,
                "illumination": 0
            }
        }
        
        Args:
            data: JSON data dictionary from firmware
        """
        try:
            # Update trigger statistics
            self._trigger_count += 1
            self._last_trigger_time = time.time()
            
            # Extract trigger information
            cam_data = data.get("cam", {})
            
            # Handle simple trigger ({"cam": 1})
            if isinstance(cam_data, (int, float)):
                trigger_info = {
                    "trigger": int(cam_data),
                    "frame_id": self._trigger_count,
                    "timestamp": self._last_trigger_time
                }
                self._parent.logger.debug(f"Camera trigger received: {trigger_info}")
            else:
                # Handle extended trigger data
                trigger_info = {
                    "trigger": cam_data.get("trigger", 1),
                    "frame_id": cam_data.get("frame_id", self._trigger_count),
                    "illumination": cam_data.get("illumination", -1),
                    "timestamp": self._last_trigger_time
                }
                self._parent.logger.debug(f"Camera trigger with data received: {trigger_info}") 
            
            # Call all registered callbacks
            for key, callback in self._callbackPerKey.items():
                if callback is not None and callable(callback):
                    try:
                        callback(trigger_info)
                    except Exception as callback_error:
                        print(f"Error in camera trigger callback {key}: {callback_error}")
                        
        except Exception as e:
            print(f"Error in _callback_camera_trigger: {e}")

    def init_callback_functions(self, nCallbacks=10):
        """
        Initialize callback function dictionary.
        
        Args:
            nCallbacks: Number of callback slots to create
        """
        self._callbackPerKey = {}
        self.nCallbacks = nCallbacks
        for i in range(nCallbacks):
            self._callbackPerKey[i] = None

    def register_callback(self, key, callback):
        """
        Register a callback function for camera trigger events.
        
        Args:
            key: Integer key (0 to nCallbacks-1) for this callback
            callback: Function to call when trigger is received.
                     Function signature: callback(trigger_info: dict)
                     
        Example:
            def on_camera_trigger(info):
                print(f"Frame {info['frame_id']} triggered at {info['timestamp']}")
                camera.snap_image()
                
            ESP32.camera_trigger.register_callback(0, on_camera_trigger)
        """
        if key < 0 or key >= self.nCallbacks:
            raise ValueError(f"Callback key must be between 0 and {self.nCallbacks-1}")
        self._callbackPerKey[key] = callback

    def unregister_callback(self, key):
        """
        Remove a registered callback.
        
        Args:
            key: Integer key of callback to remove
        """
        if key in self._callbackPerKey:
            self._callbackPerKey[key] = None

    def clear_all_callbacks(self):
        """Remove all registered callbacks."""
        self.init_callback_functions(self.nCallbacks)

    def get_trigger_count(self):
        """
        Get the total number of triggers received since initialization.
        
        Returns:
            int: Number of camera triggers received
        """
        return self._trigger_count

    def get_last_trigger_time(self):
        """
        Get the timestamp of the last trigger received.
        
        Returns:
            float or None: Unix timestamp of last trigger, or None if no triggers received
        """
        return self._last_trigger_time

    def reset_trigger_count(self):
        """Reset the trigger counter to zero."""
        self._trigger_count = 0
        self._last_trigger_time = None

    def get_trigger_stats(self):
        """
        Get statistics about trigger events.
        
        Returns:
            dict: Dictionary with trigger statistics
        """
        return {
            "total_triggers": self._trigger_count,
            "last_trigger_time": self._last_trigger_time,
            "callbacks_registered": sum(1 for cb in self._callbackPerKey.values() if cb is not None)
        }
