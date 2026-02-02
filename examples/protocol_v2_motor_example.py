"""
Example: Using Protocol V2 with Motor Control

This example demonstrates how to use the new SerialCommunicator with
the Motor class for high-level motor control with MessagePack optimization.
"""

from uc2rest.mserial_v2 import SerialCommunicator
import time


def example_basic_usage():
    """Basic motor control with new protocol"""
    print("=== Basic Motor Control Example ===\n")
    
    # Create communicator with MessagePack enabled
    comm = SerialCommunicator(
        port='/dev/ttyUSB0',
        baudrate=115200,
        use_msgpack=True,
        debug=True  # See protocol details
    )
    
    try:
        # Simple state query
        print("1. Getting device state...")
        response = comm.send_command(
            {"task": "/state_get"},
            blocking=True,
            timeout=3.0
        )
        print(f"Response: {response}\n")
        
        # Move motor (blocking - wait for completion)
        print("2. Moving motor X to position 1000...")
        move_cmd = {
            "task": "/motor_act",
            "motor": {
                "steppers": [{
                    "stepperid": 1,  # X-axis
                    "position": 1000,
                    "speed": 10000,
                    "isabs": 0,
                    "isaccel": 0
                }]
            }
        }
        response = comm.send_command(move_cmd, blocking=True, timeout=5.0)
        print(f"Move completed: {response}\n")
        
        # Non-blocking motor move
        print("3. Starting non-blocking move...")
        move_cmd["motor"]["steppers"][0]["position"] = -1000
        comm.send_command(move_cmd, blocking=False)
        print("Command sent, continuing...\n")
        
        # Configure hard limits
        print("4. Enabling hard limits...")
        hardlimit_cmd = {
            "task": "/motor_act",
            "hardlimits": {
                "steppers": [{
                    "stepperid": 1,
                    "enabled": 1,
                    "polarity": 0
                }]
            }
        }
        response = comm.send_command(hardlimit_cmd, blocking=True)
        print(f"Hard limits configured: {response}\n")
        
        # Show statistics
        stats = comm.get_statistics()
        print("5. Communication Statistics:")
        print(f"   Messages sent: {stats['messages_sent']}")
        print(f"   Messages received: {stats['messages_received']}")
        print(f"   Timeouts: {stats['timeouts']}")
        print(f"   Errors: {stats['errors']}")
        
    finally:
        comm.close()


def example_callback_notifications():
    """Example using callbacks for async notifications"""
    print("\n=== Callback Notifications Example ===\n")
    
    comm = SerialCommunicator(
        port='/dev/ttyUSB0',
        use_msgpack=True,
        debug=False
    )
    
    # Track position updates
    position_updates = []
    
    def on_position_update(data):
        """Called when position data is received"""
        if "position" in str(data):
            position_updates.append(data)
            print(f"Position update: {data}")
    
    # Register callback
    comm.register_callback(on_position_update, pattern="position")
    
    try:
        # Start a long scanning operation
        print("Starting scan with position notifications...")
        scan_cmd = {
            "task": "/scanner_start",
            "params": {"notify_position": True}
        }
        comm.send_command(scan_cmd, blocking=False)
        
        # Wait for updates
        time.sleep(5)
        
        print(f"\nReceived {len(position_updates)} position updates")
        
    finally:
        comm.close()


def example_performance_comparison():
    """Compare MessagePack vs JSON performance"""
    print("\n=== Performance Comparison ===\n")
    
    # Test command (typical motor move)
    test_command = {
        "task": "/motor_act",
        "motor": {
            "steppers": [{
                "stepperid": 1,
                "position": 1000,
                "speed": 15000,
                "isabs": 0,
                "isaccel": 0
            }]
        }
    }
    
    iterations = 50
    
    # Test with MessagePack
    print("Testing with MessagePack...")
    comm_mp = SerialCommunicator(port='/dev/ttyUSB0', use_msgpack=True, debug=False)
    
    t0 = time.time()
    for i in range(iterations):
        comm_mp.send_command(test_command, blocking=True, timeout=2.0)
        if i % 10 == 0:
            print(f"  Progress: {i}/{iterations}")
    msgpack_time = time.time() - t0
    
    stats_mp = comm_mp.get_statistics()
    comm_mp.close()
    
    # Test with JSON
    print("\nTesting with JSON...")
    comm_json = SerialCommunicator(port='/dev/ttyUSB0', use_msgpack=False, debug=False)
    
    t0 = time.time()
    for i in range(iterations):
        comm_json.send_command(test_command, blocking=True, timeout=2.0)
        if i % 10 == 0:
            print(f"  Progress: {i}/{iterations}")
    json_time = time.time() - t0
    
    stats_json = comm_json.get_statistics()
    comm_json.close()
    
    # Results
    print("\n--- Results ---")
    print(f"MessagePack time: {msgpack_time:.2f}s ({msgpack_time/iterations*1000:.1f}ms per command)")
    print(f"JSON time:        {json_time:.2f}s ({json_time/iterations*1000:.1f}ms per command)")
    print(f"Speedup:          {json_time/msgpack_time:.2f}x faster with MessagePack")
    print(f"\nMessages sent (MP):   {stats_mp['messages_sent']}")
    print(f"Messages sent (JSON): {stats_json['messages_sent']}")
    print(f"Timeouts (MP):        {stats_mp['timeouts']}")
    print(f"Timeouts (JSON):      {stats_json['timeouts']}")


def example_motor_wrapper():
    """Example of wrapping SerialCommunicator in Motor class"""
    
    class MotorV2:
        """Modern motor controller using Protocol V2"""
        
        def __init__(self, comm: SerialCommunicator):
            self.comm = comm
        
        def move(self, axis, position, speed=10000, absolute=False, blocking=True):
            """Move motor to position"""
            # Convert axis name to ID
            axis_map = {'A': 0, 'X': 1, 'Y': 2, 'Z': 3}
            axis_id = axis_map.get(axis.upper(), axis) if isinstance(axis, str) else axis
            
            command = {
                "task": "/motor_act",
                "motor": {
                    "steppers": [{
                        "stepperid": axis_id,
                        "position": position,
                        "speed": speed,
                        "isabs": 1 if absolute else 0,
                        "isaccel": 0
                    }]
                }
            }
            
            return self.comm.send_command(command, blocking=blocking, timeout=10.0)
        
        def enable_hard_limits(self, axis, enabled=True, polarity=0):
            """Enable/disable hard limit protection"""
            axis_map = {'A': 0, 'X': 1, 'Y': 2, 'Z': 3}
            axis_id = axis_map.get(axis.upper(), axis) if isinstance(axis, str) else axis
            
            command = {
                "task": "/motor_act",
                "hardlimits": {
                    "steppers": [{
                        "stepperid": axis_id,
                        "enabled": 1 if enabled else 0,
                        "polarity": polarity
                    }]
                }
            }
            
            return self.comm.send_command(command, blocking=True, timeout=3.0)
        
        def get_position(self, axis=None):
            """Get current motor position(s)"""
            response = self.comm.send_command(
                {"task": "/motor_get"},
                blocking=True,
                timeout=3.0
            )
            
            if response and "steppers" in response:
                if axis is not None:
                    axis_map = {'A': 0, 'X': 1, 'Y': 2, 'Z': 3}
                    axis_id = axis_map.get(axis.upper(), axis) if isinstance(axis, str) else axis
                    
                    for stepper in response["steppers"]:
                        if stepper.get("stepperid") == axis_id:
                            return stepper.get("position")
                else:
                    return response["steppers"]
            
            return None
        
        def home(self, axis, direction=1, speed=5000):
            """Home a motor axis"""
            axis_map = {'A': 0, 'X': 1, 'Y': 2, 'Z': 3}
            axis_id = axis_map.get(axis.upper(), axis) if isinstance(axis, str) else axis
            
            command = {
                "task": "/home_act",
                "home": {
                    "steppers": [{
                        "stepperid": axis_id,
                        "direction": direction,
                        "speed": speed,
                        "timeout": 20
                    }]
                }
            }
            
            return self.comm.send_command(command, blocking=True, timeout=25.0)
    
    # Usage example
    print("\n=== Motor Wrapper Example ===\n")
    
    comm = SerialCommunicator(port='/dev/ttyUSB0', use_msgpack=True)
    motor = MotorV2(comm)
    
    try:
        # Enable hard limits
        print("Enabling hard limits for X-axis...")
        motor.enable_hard_limits('X', enabled=True, polarity=0)
        
        # Move motor
        print("Moving X-axis to position 1000...")
        motor.move('X', position=1000, speed=10000, blocking=True)
        
        # Get position
        position = motor.get_position('X')
        print(f"Current X position: {position}")
        
        # Home axis
        print("Homing X-axis...")
        motor.home('X', direction=-1, speed=5000)
        
    finally:
        comm.close()


if __name__ == "__main__":
    print("Serial Protocol V2 - Motor Control Examples")
    print("=" * 50)
    
    # Run examples (comment out as needed)
    example_basic_usage()
    # example_callback_notifications()
    # example_performance_comparison()
    # example_motor_wrapper()
    
    print("\n✓ Examples completed")
