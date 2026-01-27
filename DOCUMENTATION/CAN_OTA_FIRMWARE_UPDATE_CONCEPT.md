# CAN-Based OTA Firmware Update via ISO-TP Protocol

## Executive Summary

This document describes the implementation of firmware updates over CAN bus using the ISO-TP protocol for UC2 ESP32 devices. This enables firmware updates without requiring WiFi connectivity on slave devices.

**Status: IMPLEMENTED** ✅

## Implementation Summary

### Files Modified/Created

**ESP32 Firmware (uc2-ESP):**
- `main/src/can/CanOtaTypes.h` - Data structures and constants
- `main/src/can/CanOtaHandler.h` - Function declarations
- `main/src/can/CanOtaHandler.cpp` - Core OTA handler implementation
- `main/src/can/can_messagetype.h` - Added OTA_CAN_* message types (0x62-0x69)
- `main/src/can/can_controller.h` - Added actCanOta() declaration
- `main/src/can/can_controller.cpp` - Integration with dispatchIsoTpData
- `main/src/serial/SerialProcess.cpp` - Added /can_ota endpoint
- `main/src/wifi/Endpoints.h` - Added can_ota_endpoint
- `main/CMakeLists.txt` - Added CanOtaHandler.cpp to build

**Python Client (UC2-REST):**
- `uc2rest/can_ota_direct.py` - Python client class
- `uc2rest/test_can_ota_direct.py` - Standalone test script

## Current Architecture

### Existing OTA Implementation (WiFi-based)
The current system uses a two-step process:
1. **Master sends OTA command** via CAN to slave with WiFi credentials
2. **Slave connects to WiFi** and starts ArduinoOTA server
3. **PC uploads firmware** via WiFi to the slave device

**Advantages:**
- High bandwidth (~100KB/s via WiFi)
- Proven, reliable (uses ArduinoOTA library)

**Disadvantages:**
- Requires WiFi network
- Slave devices need WiFi capability
- Network configuration complexity

---

## CAN-Based OTA Architecture (NEW)

### Data Flow
```
┌──────────────┐     Serial      ┌─────────────────┐      CAN/ISO-TP      ┌──────────────┐
│   Python     │ ──────────────► │  Master ESP32   │ ──────────────────► │  Slave ESP32 │
│   Script     │  JSON Commands  │ UC2_3_HAT_MASTER│  Firmware Chunks    │ (Motor/LED)  │
│   + Binary   │                 │                 │                     │              │
└──────────────┘                 └─────────────────┘                     └──────────────┘
```

### Protocol Stack
```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│  (OTA Protocol: Start, Data, Verify, Finish, Abort)         │
├─────────────────────────────────────────────────────────────┤
│                    Transport Layer                          │
│           ISO-TP (ISO 15765-2) - Segmentation               │
├─────────────────────────────────────────────────────────────┤
│                    Data Link Layer                          │
│              CAN 2.0B (TWAI on ESP32)                       │
└─────────────────────────────────────────────────────────────┘
```

---

## Technical Analysis

### 1. ISO-TP Implementation Review

**Current Implementation** ([CanIsoTp.cpp](../../../uc2-ESP/main/src/can/iso-tp-twai/CanIsoTp.cpp)):

| Feature | Current Status | OTA Requirement |
|---------|----------------|-----------------|
| Max Message Length | 4095 bytes (12-bit length in FF) | ❌ Insufficient (~300KB-1MB firmware) |
| Flow Control | ✅ Implemented | ✅ OK |
| Consecutive Frames | ✅ Implemented | ✅ OK |
| Error Handling | Basic timeout | ⚠️ Needs enhancement |
| CRC/Checksum | ❌ Not implemented | ❌ Must add |

**Critical Limitation:** The current ISO-TP implementation uses a 12-bit length field in the First Frame (FF), limiting maximum message size to 4095 bytes. Firmware binaries are typically 300KB-1MB.

### 2. Proposed Solutions

#### Option A: Extended ISO-TP with Chunking Protocol
Add an application-layer chunking protocol on top of ISO-TP:

```
Firmware (1MB) → Chunks (4KB each) → ISO-TP messages → CAN frames
                     ↓
              256 chunks × 4KB = 1MB
```

**Chunk Protocol:**
```c
typedef struct {
    uint8_t messageType;    // OTA_DATA = 0x62
    uint16_t chunkIndex;    // 0-65535
    uint16_t totalChunks;   // Total number of chunks
    uint32_t chunkCRC32;    // CRC32 of this chunk
    uint8_t data[4088];     // Chunk data (4KB - header)
} OtaChunk;
```

#### Option B: Streaming Protocol (Direct ISO-TP Bypass)
Bypass ISO-TP's length limitation by implementing a custom streaming protocol:

```c
typedef struct {
    uint8_t frameType;      // START, DATA, END, ACK, NAK
    uint8_t sequenceNum;    // 0-255 rolling
    uint8_t data[6];        // Payload
} OtaStreamFrame;
```

### 3. Recommended Architecture (Option A)

#### New Message Types
```c
// Add to can_messagetype.h
enum CANMessageTypeID {
    // ... existing types ...
    
    // CAN OTA Update Messages (new)
    OTA_CAN_START    = 0x62,  // Start CAN-based OTA
    OTA_CAN_DATA     = 0x63,  // Firmware data chunk
    OTA_CAN_VERIFY   = 0x64,  // Request verification
    OTA_CAN_FINISH   = 0x65,  // Finish and reboot
    OTA_CAN_ABORT    = 0x66,  // Abort OTA
    OTA_CAN_ACK      = 0x67,  // Acknowledgment
    OTA_CAN_NAK      = 0x68,  // Negative acknowledgment
    OTA_CAN_STATUS   = 0x69,  // Status request/response
};
```

#### OTA Protocol State Machine

```
                    ┌─────────────┐
                    │    IDLE     │
                    └──────┬──────┘
                           │ OTA_CAN_START
                           ▼
                    ┌─────────────┐
              ┌─────│  RECEIVING  │◄────┐
              │     └──────┬──────┘     │
              │            │            │ OTA_CAN_DATA
    OTA_CAN_ABORT          │            │ (more chunks)
              │            │            │
              │            ▼            │
              │     ┌─────────────┐     │
              │     │ CHUNK_RECV  │─────┘
              │     └──────┬──────┘
              │            │ All chunks received
              │            ▼
              │     ┌─────────────┐
              │     │  VERIFYING  │
              │     └──────┬──────┘
              │            │ OTA_CAN_VERIFY (MD5 OK)
              │            ▼
              │     ┌─────────────┐
              │     │  FINISHING  │
              │     └──────┬──────┘
              │            │ OTA_CAN_FINISH
              │            ▼
              │     ┌─────────────┐
              └────►│   REBOOT    │
                    └─────────────┘
```

---

## Implementation Plan

### Phase 1: ESP32 Slave Modifications

#### 1.1 New OtaTypes.h Structures
```c
// CAN OTA Configuration
#define OTA_CHUNK_SIZE          2048    // 2KB chunks (fits in ISO-TP)
#define OTA_MAX_FIRMWARE_SIZE   0x140000 // 1.25MB (from partition table)

typedef struct {
    uint32_t firmwareSize;      // Total firmware size in bytes
    uint32_t totalChunks;       // Number of chunks
    uint32_t chunkSize;         // Size of each chunk
    uint8_t md5Hash[16];        // MD5 hash of entire firmware
    uint8_t partitionLabel[16]; // Target partition (e.g., "app0" or "app1")
} OtaCanStartCommand;

typedef struct {
    uint16_t chunkIndex;        // Current chunk index (0-based)
    uint16_t chunkSize;         // Actual size of this chunk
    uint32_t crc32;             // CRC32 of chunk data
    uint8_t data[];             // Variable length chunk data
} OtaCanDataChunk;

typedef struct {
    uint8_t status;             // 0=OK, 1=CRC error, 2=sequence error, etc.
    uint16_t expectedChunk;     // Next expected chunk index
    uint32_t bytesReceived;     // Total bytes received so far
} OtaCanAck;
```

#### 1.2 Slave OTA Handler (can_controller.cpp)
```c
// State variables
static bool canOtaActive = false;
static uint32_t canOtaFirmwareSize = 0;
static uint32_t canOtaTotalChunks = 0;
static uint32_t canOtaNextChunk = 0;
static uint8_t canOtaMd5[16];
static esp_ota_handle_t canOtaHandle;
static const esp_partition_t* canOtaPartition;

void handleCanOtaStart(OtaCanStartCommand* cmd);
void handleCanOtaData(OtaCanDataChunk* chunk);
void handleCanOtaVerify();
void handleCanOtaFinish();
void handleCanOtaAbort();
```

### Phase 2: ESP32 Master Modifications

#### 2.1 Serial JSON Command Interface
```json
{
    "task": "/can_ota",
    "canid": 11,
    "action": "start",
    "firmware_size": 524288,
    "chunk_size": 2048,
    "md5": "d41d8cd98f00b204e9800998ecf8427e"
}

{
    "task": "/can_ota",
    "canid": 11,
    "action": "data",
    "chunk_index": 0,
    "chunk_crc": 3456789012,
    "data_base64": "SGVsbG8gV29ybGQ..."
}
```

#### 2.2 Master Relay Function
```c
int relayOtaChunkToSlave(uint8_t slaveId, OtaCanDataChunk* chunk) {
    uint8_t buffer[sizeof(CANMessageTypeID) + sizeof(OtaCanDataChunk) + chunk->chunkSize];
    buffer[0] = OTA_CAN_DATA;
    memcpy(&buffer[1], chunk, sizeof(OtaCanDataChunk) + chunk->chunkSize);
    return sendCanMessage(slaveId, buffer, sizeof(buffer));
}
```

### Phase 3: Python Client Implementation

#### 3.1 UC2REST canota.py Extension
```python
class CANOTADirect:
    """Direct CAN-based OTA (no WiFi required)"""
    
    CHUNK_SIZE = 2048
    
    def upload_firmware(self, can_id: int, firmware_path: str, 
                       progress_callback=None) -> bool:
        """
        Upload firmware to CAN device via ISO-TP.
        
        Args:
            can_id: Target device CAN ID
            firmware_path: Path to .bin firmware file
            progress_callback: Optional callback(bytes_sent, total_bytes)
        """
        # Read firmware file
        with open(firmware_path, 'rb') as f:
            firmware = f.read()
        
        total_size = len(firmware)
        md5_hash = hashlib.md5(firmware).hexdigest()
        num_chunks = (total_size + self.CHUNK_SIZE - 1) // self.CHUNK_SIZE
        
        # Send OTA_CAN_START
        start_cmd = {
            "task": "/can_ota",
            "canid": can_id,
            "action": "start",
            "firmware_size": total_size,
            "chunk_size": self.CHUNK_SIZE,
            "total_chunks": num_chunks,
            "md5": md5_hash
        }
        response = self._parent.post_json("/can_ota", start_cmd)
        
        if not self._check_ack(response):
            return False
        
        # Send chunks
        for i in range(num_chunks):
            chunk_start = i * self.CHUNK_SIZE
            chunk_end = min(chunk_start + self.CHUNK_SIZE, total_size)
            chunk_data = firmware[chunk_start:chunk_end]
            chunk_crc = zlib.crc32(chunk_data) & 0xFFFFFFFF
            
            data_cmd = {
                "task": "/can_ota",
                "canid": can_id,
                "action": "data",
                "chunk_index": i,
                "chunk_crc": chunk_crc,
                "data_base64": base64.b64encode(chunk_data).decode()
            }
            
            response = self._parent.post_json("/can_ota", data_cmd)
            
            if not self._check_ack(response):
                # Retry logic...
                pass
            
            if progress_callback:
                progress_callback(chunk_end, total_size)
        
        # Send verify & finish
        # ...
```

---

## Data Integrity Mechanisms

### 1. Per-Chunk CRC32
- Each 2KB chunk includes a CRC32 checksum
- Slave verifies CRC before writing to flash
- NAK sent if CRC mismatch → chunk retransmission

### 2. Full Firmware MD5
- MD5 hash calculated on PC before upload
- Slave calculates MD5 after receiving all chunks
- OTA_CAN_VERIFY compares hashes
- Prevents flash write if mismatch

### 3. Sequence Verification
- Chunk index tracked on slave
- Out-of-order chunks rejected
- Missing chunks can be re-requested

### 4. Flash Verification
- ESP32's `Update` library performs internal verification
- Uses `Update.setMD5()` for automatic verification

---

## Performance Estimates

### CAN Bus Throughput
- CAN 2.0B at 500 kbps (typical UC2 configuration)
- Effective data rate after overhead: ~40 KB/s
- **Estimated transfer time for 512KB firmware: ~13 seconds**

### Comparison
| Method | Transfer Time (512KB) | Requirements |
|--------|----------------------|--------------|
| WiFi OTA | ~5 seconds | WiFi network |
| CAN ISO-TP | ~13 seconds | CAN only |
| USB Serial | ~1 minute | Physical access |

---

## Implementation Complexity

### Code Changes Required

| Component | Lines of Code | Difficulty |
|-----------|---------------|------------|
| OtaTypes.h (new structs) | ~80 | Easy |
| can_controller.cpp (slave handler) | ~300 | Medium |
| can_controller.cpp (master relay) | ~150 | Medium |
| SerialProcess.cpp (JSON handler) | ~100 | Easy |
| Python canota.py extension | ~200 | Easy |
| **Total** | **~830** | **Medium** |

---

## Partition Table Requirements

The ESP32S3 slave devices use:
```csv
# custom_partition_esp32s3.csv
app0,     app,  ota_0,   0x10000, 0x140000   # 1.25MB
app1,     app,  ota_1,   0x150000,0x140000   # 1.25MB
```

This supports OTA updates with A/B partition switching ✅

---

## Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Transfer corruption | Low | High | CRC32 + MD5 verification |
| Buffer overflow | Medium | High | Strict size checks |
| Incomplete transfer | Medium | Medium | State machine + retry logic |
| Flash write failure | Low | High | Use ESP32 Update library |
| CAN bus congestion | Medium | Low | Flow control, priority handling |

---

## Recommended Next Steps

1. **Proof of Concept** (1-2 days)
   - Implement basic chunk transfer without flash write
   - Test ISO-TP with 2KB payloads

2. **Slave Implementation** (2-3 days)
   - Add OTA_CAN_* message handlers
   - Integrate with ESP32 Update library

3. **Master Implementation** (1-2 days)
   - Add JSON command parsing
   - Implement relay functionality

4. **Python Client** (1 day)
   - Extend canota.py
   - Add progress reporting

5. **Testing & Validation** (2-3 days)
   - Full transfer tests
   - Error recovery tests
   - Performance benchmarking

**Total estimated time: 7-11 days**

---

## Conclusion

CAN-based OTA firmware updates are **feasible** with the existing UC2 architecture. The main challenges are:

1. **ISO-TP length limitation** → Solved with application-layer chunking
2. **Data integrity** → Solved with CRC32 + MD5
3. **Transfer reliability** → Solved with ACK/NAK and retry logic

The implementation provides a valuable alternative to WiFi-based OTA, especially for:
- Environments without WiFi
- Production/factory flashing
- Automated batch updates

---

## References

- [ISO 15765-2 (ISO-TP)](https://en.wikipedia.org/wiki/ISO_15765-2)
- [ESP32 Update Library](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/update.html)
- [ArduinoOTA Implementation](ArduinoOTA.cpp) (attached)
- [Current CAN Controller](can_controller.cpp) (attached)
- [ISO-TP Implementation](CanIsoTp.cpp) (attached)
