# Migration Guide: Upgrading to Serial Protocol V2

## Overview

This guide helps you migrate from the legacy JSON serial communication to the new MessagePack-enabled Protocol V2.

## Migration Steps

### For Python Users

#### Step 1: Install msgpack

```bash
pip install msgpack
```

#### Step 2: Update your code (Option A - Drop-in replacement)

The new `mserial_v2.py` is backward compatible with the old `mserial.py`:

```python
# Old code
from uc2rest.mserial import Serial
serial = Serial(port='/dev/ttyUSB0')

# New code - simply change import
from uc2rest.mserial_v2 import Serial  # or SerialCommunicator
serial = Serial(port='/dev/ttyUSB0')  # Works the same!
```

#### Step 3: Update your code (Option B - Use new API)

For better clarity, use the new `SerialCommunicator` class:

```python
from uc2rest.mserial_v2 import SerialCommunicator

# Initialize with MessagePack support (default)
comm = SerialCommunicator(
    port='/dev/ttyUSB0',
    baudrate=115200,
    use_msgpack=True,  # Enable MessagePack (default)
    debug=False         # Enable debug logging if needed
)

# Blocking command (wait for response)
response = comm.send_command(
    {"task": "/state_get"},
    blocking=True,
    timeout=5.0
)
print(f"Response: {response}")

# Non-blocking command (fire and forget)
comm.send_command(
    {"task": "/led_set", "led": 100},
    blocking=False
)

# Close when done
comm.close()
```

#### Step 4: Update Motor class usage

The Motor class will work transparently with the new protocol:

```python
from uc2rest import UC2Client

# Create client (will use new protocol automatically)
client = UC2Client(serialport='/dev/ttyUSB0')

# All existing methods work unchanged
client.motor.enable_hard_limits(axis="X", enabled=True)
client.motor.move_x(position=1000, speed=10000)

# Check statistics (new feature)
stats = client.serial.get_statistics()
print(f"Messages sent: {stats['messages_sent']}")
print(f"Messages received: {stats['messages_received']}")
```

### For ESP32 Firmware

#### Step 1: Add mpack library

Update `platformio.ini`:

```ini
[env:esp32_framework]
lib_deps = 
    teemuatlut/TMCStepper@^0.7.3
    madhephaestus/ESP32Encoder@^0.12.0
    ludocode/mpack@^1.1.1  # Add this line
```

#### Step 2: Enable MessagePack support

Add build flag in `platformio.ini`:

```ini
build_flags = 
    ...existing flags...
    -DHAS_MPACK  # Enable MessagePack support
```

#### Step 3: Include new headers

Update `SerialProcess.cpp`:

```cpp
#include "SerialProtocol.h"  // Add new protocol handler
```

#### Step 4: Update serial input handling

Replace JSON-only parsing:

```cpp
// OLD CODE - JSON only
void loop()
{
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        cJSON *root = cJSON_Parse(input.c_str());
        if (root != NULL) {
            processJsonDocument(root);
        }
    }
}
```

With multi-format support:

```cpp
// NEW CODE - JSON + MessagePack
static uint8_t readBuffer[8192];
static size_t bufferPos = 0;

void loop()
{
    // Read available data
    while (Serial.available() > 0) {
        readBuffer[bufferPos++] = Serial.read();
        
        // Check if message is complete
        if (SerialProtocol::isMessageComplete(readBuffer, bufferPos)) {
            // Decode message (auto-detects format)
            cJSON *root = SerialProtocol::decode(readBuffer, bufferPos);
            
            if (root != NULL) {
                processJsonDocument(root);
                // Note: processJsonDocument now deletes root
            }
            
            // Reset buffer
            bufferPos = 0;
        }
        
        // Prevent buffer overflow
        if (bufferPos >= sizeof(readBuffer)) {
            log_e("Buffer overflow, resetting");
            bufferPos = 0;
        }
    }
}
```

#### Step 5: Update output functions

Replace direct JSON serialization:

```cpp
// OLD CODE
void serialize(cJSON *doc)
{
    char *s = cJSON_PrintUnformatted(doc);
    if (s != NULL) {
        Serial.print("++\n");
        Serial.print(s);
        Serial.print("\n--\n");
        free(s);
    }
    cJSON_Delete(doc);
}
```

With protocol-aware serialization:

```cpp
// NEW CODE
static bool clientSupportsMsgPack = false;  // Track client capability

void serialize(cJSON *doc)
{
    if (doc == NULL) return;
    
    // Choose format based on client capability
    MessageFormat format = clientSupportsMsgPack ? 
                          MessageFormat::MSGPACK : MessageFormat::JSON;
    
    // Encode message
    size_t encodedLen = 0;
    uint8_t* encoded = SerialProtocol::encode(doc, format, encodedLen);
    
    if (encoded != nullptr) {
        // Send through safe output queue
        safeSendBytes(encoded, encodedLen);
        free(encoded);
    }
    
    cJSON_Delete(doc);
}

// Helper function to send raw bytes safely
void safeSendBytes(const uint8_t* data, size_t len)
{
    // Similar to safePrintln but for binary data
    SerialMessage msg;
    msg.message = (char*)malloc(len);
    if (msg.message != nullptr) {
        memcpy(msg.message, data, len);
        msg.length = len;
        xQueueSend(serialOutputQueue, &msg, pdMS_TO_TICKS(100));
    }
}
```

#### Step 6: Add capability negotiation

Enable clients to request MessagePack:

```cpp
void jsonProcessor(char *task, cJSON *jsonDocument)
{
    // ... existing task handlers ...
    
    // Add new capability handler
    if (strcmp(task, "/capabilities_get") == 0) {
        cJSON *response = cJSON_CreateObject();
        cJSON_AddNumberToObject(response, "qid", getQid(jsonDocument));
        cJSON_AddBoolToObject(response, "msgpack", true);
        cJSON_AddStringToObject(response, "protocol_version", "2.0");
        serialize(response);
        return;
    }
    
    // ... rest of handlers ...
}
```

## Compatibility Matrix

| Python Client | ESP32 Firmware | Result |
|---------------|----------------|---------|
| Legacy (old) | Legacy (old) | ✓ Works (JSON only) |
| V2 with msgpack=False | Legacy (old) | ✓ Works (JSON) |
| V2 with msgpack=True | Legacy (old) | ✓ Works (auto-fallback to JSON) |
| V2 with msgpack=True | V2 (new) | ✓ Works (MessagePack) |

## Testing Your Migration

### Test 1: Basic Communication

```python
from uc2rest.mserial_v2 import SerialCommunicator

comm = SerialCommunicator(port='/dev/ttyUSB0', debug=True)

# Send test command
response = comm.send_command({"task": "/state_get"}, blocking=True)
print(f"Success: {response is not None}")
```

### Test 2: Verify MessagePack Usage

Enable debug logging to see which protocol is used:

```python
comm = SerialCommunicator(port='/dev/ttyUSB0', debug=True, use_msgpack=True)

# You should see log messages like:
# [SerialProtocol] Encoded MessagePack: 83 bytes (payload: 79)
# [SerialProtocol] Decoded MessagePack: 79 bytes
```

### Test 3: Performance Comparison

```python
import time
from uc2rest.mserial_v2 import SerialCommunicator

# Test with MessagePack
comm_mp = SerialCommunicator(port='/dev/ttyUSB0', use_msgpack=True)
command = {"task": "/motor_act", "motor": {...}}  # Your command

t0 = time.time()
for _ in range(100):
    comm_mp.send_command(command, blocking=True, timeout=1.0)
msgpack_time = time.time() - t0

# Test with JSON
comm_json = SerialCommunicator(port='/dev/ttyUSB0', use_msgpack=False)
t0 = time.time()
for _ in range(100):
    comm_json.send_command(command, blocking=True, timeout=1.0)
json_time = time.time() - t0

print(f"MessagePack: {msgpack_time:.2f}s")
print(f"JSON: {json_time:.2f}s")
print(f"Speedup: {json_time/msgpack_time:.1f}x")
```

## Common Issues

### Issue: "msgpack not found"

**Solution:**
```bash
pip install msgpack
```

### Issue: ESP32 doesn't respond

**Solution:** Check that ESP32 firmware is updated and has MessagePack support compiled in. Verify build flags include `-DHAS_MPACK`.

### Issue: Getting timeouts

**Solution:** 
1. Check serial connection
2. Verify baud rate matches (115200)
3. Enable debug logging to see what's being sent/received
4. Try with `use_msgpack=False` to test JSON fallback

### Issue: Binary data corruption

**Solution:** 
1. Ensure serial port timeout settings are correct
2. Check for other processes accessing the serial port
3. Verify ESP32 output queue is handling binary data correctly

## Rollback Plan

If you need to revert:

1. **Python:** Simply use the old import:
   ```python
   from uc2rest.mserial import Serial  # Old version
   ```

2. **ESP32:** Remove `SerialProtocol.h/cpp` and revert to old `SerialProcess.cpp`

3. **Both:** Keep both implementations side-by-side during migration period

## Performance Expectations

Expected improvements with MessagePack:

- **Message size:** 25-35% reduction
- **Parsing speed:** 2-5x faster (ESP32)
- **Transmission time:** 25-35% reduction
- **Memory usage:** Similar or slightly lower

## Next Steps

After successful migration:

1. Monitor system performance and error rates
2. Gradually enable MessagePack for all devices
3. Consider Phase 2 enhancements:
   - CRC checksums for error detection
   - Compression for very large messages
   - Binary streaming for continuous data
   - asyncio integration for Python

## Support

For issues or questions:
- Check documentation: `SERIAL_PROTOCOL_V2.md`
- Review examples: `examples/protocol_v2/`
- Enable debug logging for diagnostics
- Check GitHub issues/discussions
