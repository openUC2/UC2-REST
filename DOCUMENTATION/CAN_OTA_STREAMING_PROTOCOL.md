# CAN OTA Streaming Protocol Documentation

## Overview

The CAN OTA Streaming protocol is a high-speed firmware update mechanism that enables fast over-the-air updates to CAN slave devices through the master ESP32. Unlike the traditional WiFi-based OTA, this protocol streams binary firmware data directly over Serial → CAN bus.

## Architecture

```
┌─────────────┐      Serial       ┌─────────────┐      CAN Bus      ┌─────────────┐
│   Python    │  ──────────────►  │   Master    │  ──────────────►  │   Slave     │
│   Client    │ ◄──────────────   │   ESP32     │ ◄──────────────.  │   ESP32     │
│             │  Binary Packets   │             │  ISO-TP Msgs      │             │
└─────────────┘                   └─────────────┘                   └─────────────┘
```

## Protocol Flow

### Phase 1: JSON Initialization

1. **Python → Master (JSON)**:
```json
{
    "task": "/can_ota_stream",
    "canid": 11,
    "action": "start",
    "firmware_size": 876784,
    "page_size": 4096,
    "chunk_size": 512,
    "md5": "43ba96b4d18c010201762b840476bf83"
}
```

2. **Master → Slave (CAN)**: `STREAM_START` message with firmware parameters

3. **Slave → Master (CAN)**: `STREAM_ACK` confirming ready state

4. **Master → Python (JSON)**:
```json
{"success": true, "qid": 1}
```

5. **Master enters Binary Mode**: Now expects binary packets from Serial

### Phase 2: Binary Data Transfer

The Python client sends binary packets, which the Master parses and forwards to the Slave via CAN.

#### Binary Packet Format (Serial → Master)

```
┌──────────┬──────────┬──────────┬───────────┬──────────┬───────────┬──────────┬──────────────────┬──────────┐
│  SYNC_1  │  SYNC_2  │   CMD    │ pageIndex │  offset  │  dataSize │   seq    │      DATA        │ CHECKSUM │
│   0xAA   │   0x55   │  (1B)    │   (2B)    │   (2B)   │   (2B)    │   (2B)   │   (dataSize)     │   (1B)   │
└──────────┴──────────┴──────────┴───────────┴──────────┴───────────┴──────────┴──────────────────┴──────────┘
```

**Important**: Multi-byte fields are **Big Endian** from Python.

#### Commands

| Command        | Value | Description                      |
|----------------|-------|----------------------------------|
| STREAM_START   | 0x70  | Initialize streaming session     |
| STREAM_DATA    | 0x71  | Data frame with sequence number  |
| STREAM_ACK     | 0x72  | Cumulative acknowledgment        |
| STREAM_NAK     | 0x73  | Request retransmit from offset   |
| STREAM_FINISH  | 0x74  | End session, verify, commit      |
| STREAM_ABORT   | 0x75  | Abort session                    |
| STREAM_STATUS  | 0x76  | Query status                     |

### Phase 3: ACK/NAK Responses

When the Slave acknowledges a page, it sends `STREAM_ACK` to the Master via CAN. The Master forwards this as a binary packet to Python:

#### Binary ACK Format (Master → Python)

```
┌──────────┬──────────┬─────────────┬──────────┬────────────┬───────────────┬────────────────┬──────────┬──────────┐
│  SYNC_1  │  SYNC_2  │ STREAM_ACK  │  status  │   canId    │lastCompletePg │ bytesReceived  │ nextSeq  │ CHECKSUM │
│   0xAA   │   0x55   │    0x72     │   (1B)   │   (1B)     │    (2B LE)    │    (4B LE)     │ (2B LE)  │   (1B)   │
└──────────┴──────────┴─────────────┴──────────┴────────────┴───────────────┴────────────────┴──────────┴──────────┘
```

**Important**: Response multi-byte fields are **Little Endian** (matching ESP32 native format).

### Phase 4: Finish & Verify

After all pages are sent:

1. **Python → Master (Binary)**: `STREAM_FINISH` packet with MD5 hash
2. **Master → Slave (CAN)**: Forward finish command
3. **Slave verifies MD5**, writes final sector if needed
4. **Slave → Master (CAN)**: `STREAM_ACK` (success) or `STREAM_NAK` (MD5 mismatch)
5. **Master → Python (Binary)**: Forward response
6. **Slave reboots** with new firmware

## Implementation Details

### Master (ESP32) Components

#### CanOtaStreaming.h
- Protocol constants and structures
- Message type definitions
- Function declarations

#### CanOtaStreaming.cpp

**Key Functions (Master-side):**
- `enterStreamingBinaryMode()` - Activate binary packet parsing mode
- `processBinaryStreamPacket()` - State machine for Serial binary input
- `sendStreamData()` - Forward data to Slave via CAN
- `handleSlaveStreamResponse()` - Process ACK/NAK from Slave
- `forwardSlaveAckToSerial()` - Send binary ACK to Python

**Key Functions (Slave-side):**
- `handleStartCmd()` - Initialize OTA session
- `handleDataChunk()` - Accumulate data in page buffer
- `handleFinishCmd()` - Verify MD5, finalize update, reboot

#### SerialProcess.cpp Integration

```cpp
void loop() {
    #ifdef CAN_BUS_ENABLED
    // Check streaming binary mode first
    if (can_ota_stream::isStreamingModeActive()) {
        can_ota_stream::processBinaryStreamPacket();
        return;  // Don't process JSON in streaming binary mode
    }
    #endif
    
    // Normal JSON processing...
}
```

### Python Client

See [tools/can_ota_streaming.py](../uc2-ESP/tools/can_ota_streaming.py) for reference implementation.

**Key Functions:**
- `build_stream_data_packet()` - Create binary data packet
- `build_stream_finish_packet()` - Create finish packet with MD5
- `wait_for_stream_ack()` - Parse binary ACK response
- `upload_firmware_streaming()` - Main upload orchestration

## Performance

| Protocol               | 1MB Firmware Time | Speed       |
|------------------------|-------------------|-------------|
| WiFi OTA (ArduinoOTA)  | ~30-60 seconds    | ~17-34 KB/s |
| Old CAN (ACK per 1KB)  | ~6 minutes        | ~2.8 KB/s   |
| **Streaming CAN**      | **~1.5-2 min**    | **~8-11 KB/s** |

The streaming protocol achieves 3-4x speedup by:
- Sending 8 chunks (4KB page) before waiting for ACK
- Using async flash writes on separate FreeRTOS task
- Reducing CAN protocol overhead

## Error Handling

### Error Codes

| Code | Name                    | Description                    |
|------|-------------------------|--------------------------------|
| 0x00 | CAN_OTA_OK              | Success                        |
| 0x01 | CAN_OTA_ERR_BEGIN       | Update.begin() failed          |
| 0x02 | CAN_OTA_ERR_WRITE       | Flash write error              |
| 0x03 | CAN_OTA_ERR_CRC         | Checksum/CRC mismatch          |
| 0x04 | CAN_OTA_ERR_TIMEOUT     | Timeout waiting for data       |
| 0x05 | CAN_OTA_ERR_VERIFY      | MD5 verification failed        |
| 0x06 | CAN_OTA_ERR_INVALID_SIZE| Firmware too large             |
| 0x07 | CAN_OTA_ERR_BUSY        | Another OTA in progress        |
| 0x08 | CAN_OTA_ERR_NOT_STARTED | No active session              |
| 0x09 | CAN_OTA_ERR_SEND        | CAN send failed                |
| 0x0A | CAN_OTA_ERR_NAK         | Slave NAK'd the request        |
| 0x0B | CAN_OTA_ERR_ABORTED     | Session aborted                |

### Recovery

If an error occurs:
1. Master sends NAK to Python with error code
2. Python can abort session: `{"task": "/can_ota_stream", "action": "abort"}`
3. Both Master and Slave clean up state
4. User can retry from beginning

## Debugging

### Enable Verbose Logging

Master automatically logs all streaming events. Key log messages:

```
[I][CanOtaStreaming.cpp] Entering binary streaming mode: slaveId=0x0B, size=876784
[I][CanOtaStreaming.cpp] Slave STREAM_ACK: page=0, bytes=4096, nextSeq=8
[I][CanOtaStreaming.cpp] Page 1 written to flash (8192 bytes total)
...
[I][CanOtaStreaming.cpp] Slave ACK'd STREAM_FINISH - OTA complete!
```

### CAN Bus Health Check

Query CAN bus status:
```json
{"task": "/can_get"}
```

Returns TWAI status including error counters, queue depths, and bus state.

## Usage Example

```python
#!/usr/bin/env python3
import serial
import json
import hashlib
from pathlib import Path

PORT = "/dev/cu.SLAB_USBtoUART"
BAUD = 921600
CAN_ID = 11

firmware_data = Path("firmware.bin").read_bytes()
md5_hex = hashlib.md5(firmware_data).hexdigest()

ser = serial.Serial(PORT, BAUD, timeout=0.1)

# Step 1: Send JSON START
start_cmd = json.dumps({
    "task": "/can_ota_stream",
    "canid": CAN_ID,
    "action": "start",
    "firmware_size": len(firmware_data),
    "page_size": 4096,
    "chunk_size": 512,
    "md5": md5_hex
}) + '\n'
ser.write(start_cmd.encode())

# Wait for success response...

# Step 2: Send binary pages
# (See can_ota_streaming.py for complete implementation)

# Step 3: Send FINISH and wait for verification
```

## Files

| File | Description |
|------|-------------|
| `main/src/can/CanOtaStreaming.h` | Protocol definitions |
| `main/src/can/CanOtaStreaming.cpp` | Master/Slave implementation |
| `main/src/can/CanOtaTypes.h` | Shared type definitions |
| `main/src/serial/SerialProcess.cpp` | Binary mode integration |
| `tools/can_ota_streaming.py` | Python upload client |

## See Also

- [CAN_OTA_Documentation.md](CAN_OTA_Documentation.md) - WiFi-based OTA
- [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) - UC2 system overview
