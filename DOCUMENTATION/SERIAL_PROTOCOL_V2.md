# Serial Communication Protocol V2

## Overview

This document describes the upgraded serial communication protocol between Python host and ESP32 devices, supporting both JSON (legacy) and MessagePack (optimized) formats.

## Protocol Design Goals

1. **Efficiency**: Reduce message size with MessagePack binary serialization
2. **Backward Compatibility**: Support legacy JSON format
3. **Reliability**: Maintain QID-based request/response tracking
4. **Flexibility**: Support blocking and non-blocking operations
5. **Simplicity**: Clean, maintainable implementation

## Message Format

### Header Structure

All messages start with a 4-byte header:

```
Byte 0: Magic/Format byte
Byte 1-2: Message length (uint16, little-endian)
Byte 3: Reserved (future use, set to 0x00)
```

**Magic/Format Byte Values:**
- `0x4A` ('J'): JSON format (legacy compatibility)
- `0x4D` ('M'): MessagePack format
- `0x42` ('B'): Binary protocol (future reserved)

**Message Length**: Total payload size in bytes (excluding header)

### Message Structure

#### JSON Format (Legacy - 0x4A)

```
[0x4A][LEN_L][LEN_H][0x00]<JSON string>
```

Example:
```
4A 1E 00 00 {"task":"/state_get","qid":42}
```

#### MessagePack Format (New - 0x4D)

```
[0x4D][LEN_L][LEN_H][0x00]<MessagePack binary data>
```

The MessagePack payload contains the same logical structure as JSON:
```python
{
    "task": "/motor_act",
    "qid": 123,
    "motor": {
        "steppers": [{
            "stepperid": 1,
            "position": 1000,
            "speed": 15000
        }]
    }
}
```

### Legacy Format Support

For backward compatibility, the system continues to recognize the old delimiter-based format:

```
++\n
<JSON string>
\n--\n
```

When a message starts with `+`, the parser enters legacy mode and expects the old format.

## QID (Query ID) System

Every request should include a unique `qid` field:
- **QID > 0**: Expect response(s), blocking operation
- **QID = 0**: No response expected, fire-and-forget
- **QID < 0**: Used internally for error signaling

Responses echo the `qid` from the request.

## Communication Patterns

### Blocking Request-Response

```python
# Python sends:
{"task": "/state_get", "qid": 42}

# ESP32 responds:
{"qid": 42, "state": {...}}
```

### Non-blocking Fire-and-Forget

```python
# Python sends:
{"task": "/motor_act", "qid": 0, "motor": {...}}

# No response expected
```

### Multi-response Streaming

```python
# Python sends:
{"task": "/scanner_start", "qid": 43}

# ESP32 sends multiple responses:
{"qid": 43, "progress": 10}
{"qid": 43, "progress": 20}
{"qid": 43, "complete": true}
```

## Implementation Phases

### Phase 1: MessagePack Integration (Current)
- Add MessagePack support to Python and ESP32
- Maintain JSON fallback
- No breaking changes to API

### Phase 2: Protocol Refinement
- Optimize header format
- Add CRC/checksum for reliability
- Protocol versioning

### Phase 3: Async Refactoring
- Consider asyncio for Python side
- Improve command queue architecture
- Better error handling

## Performance Comparison

Typical command size reduction with MessagePack:

| Command Type | JSON Size | MessagePack Size | Reduction |
|--------------|-----------|------------------|-----------|
| Simple state_get | 45 bytes | 28 bytes | 38% |
| Motor command | 180 bytes | 95 bytes | 47% |
| Complex config | 520 bytes | 280 bytes | 46% |

## Compatibility Matrix

| Python Version | ESP32 Version | Format Support |
|----------------|---------------|----------------|
| Legacy | Legacy | JSON only |
| V2 | Legacy | JSON (auto-detect) |
| V2 | V2 | JSON + MessagePack |
| V3 (future) | V2 | MessagePack preferred |

## Error Handling

### Format Detection Errors
- If header is invalid, fall back to legacy JSON parsing
- Log warnings for malformed messages

### QID Mismatch
- Timeout after specified duration
- Return error to caller
- Clean up pending response queue

### Deserialization Errors
- Log error with message context
- Send error response with original QID
- Continue processing next message

## Security Considerations

1. **Message Size Limits**: Maximum 8KB per message to prevent buffer overflow
2. **QID Validation**: Ensure QID is within valid range
3. **Format Validation**: Verify magic byte before processing
4. **Timeout Protection**: All blocking operations have configurable timeouts
