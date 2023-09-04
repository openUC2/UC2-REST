#%%
import uc2rest

port = "unknown"
ESP32 = uc2rest.UC2Client(serialport=port, DEBUG=True)

# Here we record a sequence of commands and play them back 3 times
ESP32.cmdRecorder.startRecording()
if 1:
    ESP32.motor.move_x(steps=10000, speed=10000, is_blocking=True)
    ESP32.motor.move_a(steps=10000, speed=10000, is_blocking=True)
    ESP32.motor.move_z(steps=10000, speed=10000, is_blocking=True)
    ESP32.motor.move_x(steps=-10000, speed=10000, is_blocking=True)
    ESP32.led.send_LEDMatrix_full(intensity=(255, 255, 255))
    ESP32.state.delay(1000)
    ESP32.led.send_LEDMatrix_full(intensity=(0, 0, 0), getReturn=False)
    ESP32.cmdRecorder.stopRecording()
else:
    # {"tasks":[{"task":"/state_get"},{"task":"/state_act", "delay":1000}],"nTimes":2}
    ESP32.state.get_state()
    ESP32.state.delay(1000)

# Print the recorded commands
ESP32.cmdRecorder.printCommands()

# Play the recorded commands 3 times and wait until they are finished
# this way we can guarantee some kind of real-time behavior since no serial communication is involved
ESP32.cmdRecorder.playCommands(nTimes=3, isBlocking=False)


# we need to close the ESP, otherwise the thread won't stop
ESP32.close()