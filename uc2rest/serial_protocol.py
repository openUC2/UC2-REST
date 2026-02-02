"""
Serial Protocol V2 - MessagePack and JSON support

This module provides encoding/decoding for the upgraded serial protocol
supporting both MessagePack (efficient) and JSON (legacy) formats.
"""

import struct
from typing import Dict, Any, Optional, Tuple
from enum import IntEnum

try:
    import msgpack
    HAS_MSGPACK = True
except ImportError:
    HAS_MSGPACK = False
    print("WARNING: msgpack not available. Install with: pip install msgpack")

import json


class MessageFormat(IntEnum):
    """Message format identifiers"""
    JSON = 0x4A  # 'J'
    MSGPACK = 0x4D  # 'M'
    BINARY = 0x42  # 'B' (reserved for future use)


class SerialProtocol:
    """
    Handles encoding/decoding of messages for serial communication.
    
    Supports both MessagePack (preferred) and JSON (legacy) formats.
    All messages include a 4-byte header with format identifier and length.
    
    Header structure:
    - Byte 0: Magic/Format byte (0x4A=JSON, 0x4D=MessagePack)
    - Byte 1-2: Message length (uint16 little-endian)
    - Byte 3: Reserved (0x00)
    """
    
    HEADER_SIZE = 4
    MAX_MESSAGE_SIZE = 8192  # 8KB limit
    
    # Legacy JSON delimiters
    LEGACY_START = b"++\n"
    LEGACY_END = b"\n--\n"
    
    def __init__(self, prefer_msgpack: bool = True, debug: bool = False):
        """
        Initialize protocol handler.
        
        Args:
            prefer_msgpack: Use MessagePack when available (default True)
            debug: Enable debug logging (default False)
        """
        self.prefer_msgpack = prefer_msgpack and HAS_MSGPACK
        self.debug = debug
        
        if self.prefer_msgpack:
            self._log("MessagePack enabled - using efficient binary protocol")
        else:
            self._log("Using JSON protocol (msgpack not available or disabled)")
    
    def _log(self, message: str):
        """Internal logging helper"""
        if self.debug:
            print(f"[SerialProtocol] {message}")
    
    def encode(self, data: Dict[str, Any], force_json: bool = False) -> bytes:
        """
        Encode a dictionary to bytes for serial transmission.
        
        Args:
            data: Dictionary to encode (must include 'task' and 'qid' keys)
            force_json: Force JSON format even if MessagePack is available
            
        Returns:
            bytes: Encoded message with header
            
        Raises:
            ValueError: If data is invalid or too large
        """
        if not isinstance(data, dict):
            raise ValueError(f"Data must be a dictionary, got {type(data)}")
        
        if 'task' not in data:
            raise ValueError("Data must include 'task' field")
        
        # Use MessagePack if available and not forced to JSON
        use_msgpack = self.prefer_msgpack and not force_json
        
        if use_msgpack:
            return self._encode_msgpack(data)
        else:
            return self._encode_json(data)
    
    def _encode_msgpack(self, data: Dict[str, Any]) -> bytes:
        """Encode data using MessagePack format"""
        try:
            # Serialize payload
            payload = msgpack.packb(data, use_bin_type=True)
            payload_len = len(payload)
            
            if payload_len > self.MAX_MESSAGE_SIZE:
                raise ValueError(f"Message too large: {payload_len} > {self.MAX_MESSAGE_SIZE}")
            
            # Build header: [Magic][Len_L][Len_H][Reserved]
            header = struct.pack('<BHB', MessageFormat.MSGPACK, payload_len, 0)
            
            message = header + payload
            self._log(f"Encoded MessagePack: {len(message)} bytes (payload: {payload_len})")
            
            return message
            
        except Exception as e:
            self._log(f"MessagePack encoding failed: {e}, falling back to JSON")
            return self._encode_json(data)
    
    def _encode_json(self, data: Dict[str, Any]) -> bytes:
        """Encode data using JSON format"""
        try:
            # Serialize payload
            json_str = json.dumps(data, separators=(',', ':'))
            payload = json_str.encode('utf-8')
            payload_len = len(payload)
            
            if payload_len > self.MAX_MESSAGE_SIZE:
                raise ValueError(f"Message too large: {payload_len} > {self.MAX_MESSAGE_SIZE}")
            
            # Build header: [Magic][Len_L][Len_H][Reserved]
            header = struct.pack('<BHB', MessageFormat.JSON, payload_len, 0)
            
            message = header + payload
            self._log(f"Encoded JSON: {len(message)} bytes (payload: {payload_len})")
            
            return message
            
        except Exception as e:
            raise ValueError(f"JSON encoding failed: {e}")
    
    def encode_legacy(self, data: Dict[str, Any]) -> bytes:
        """
        Encode data using legacy delimiter-based JSON format.
        
        Format: ++\n<JSON>\n--\n
        
        This is kept for backward compatibility with older ESP32 firmware.
        """
        try:
            json_str = json.dumps(data, separators=(',', ':'))
            payload = json_str.encode('utf-8')
            
            message = self.LEGACY_START + payload + self.LEGACY_END
            self._log(f"Encoded legacy JSON: {len(message)} bytes")
            
            return message
            
        except Exception as e:
            raise ValueError(f"Legacy JSON encoding failed: {e}")
    
    def decode(self, data: bytes) -> Optional[Dict[str, Any]]:
        """
        Decode bytes received from serial into a dictionary.
        
        Automatically detects format (MessagePack, JSON, or legacy JSON).
        
        Args:
            data: Raw bytes from serial port
            
        Returns:
            Dictionary if successfully decoded, None otherwise
        """
        if len(data) < self.HEADER_SIZE:
            self._log(f"Message too short: {len(data)} bytes")
            return None
        
        # Check for legacy format first
        if data.startswith(self.LEGACY_START):
            return self._decode_legacy(data)
        
        # Check magic byte for new format
        magic_byte = data[0]
        
        if magic_byte == MessageFormat.MSGPACK:
            return self._decode_msgpack(data)
        elif magic_byte == MessageFormat.JSON:
            return self._decode_json(data)
        else:
            # Try legacy format as fallback
            self._log(f"Unknown magic byte: 0x{magic_byte:02X}, trying legacy format")
            return self._decode_legacy(data)
    
    def _decode_msgpack(self, data: bytes) -> Optional[Dict[str, Any]]:
        """Decode MessagePack format"""
        try:
            # Parse header
            magic, payload_len, reserved = struct.unpack('<BHB', data[:self.HEADER_SIZE])
            
            if len(data) < self.HEADER_SIZE + payload_len:
                self._log(f"Incomplete MessagePack message: expected {payload_len}, got {len(data) - self.HEADER_SIZE}")
                return None
            
            # Extract payload
            payload = data[self.HEADER_SIZE:self.HEADER_SIZE + payload_len]
            
            # Deserialize
            result = msgpack.unpackb(payload, raw=False)
            self._log(f"Decoded MessagePack: {payload_len} bytes")
            
            return result
            
        except Exception as e:
            self._log(f"MessagePack decoding failed: {e}")
            return None
    
    def _decode_json(self, data: bytes) -> Optional[Dict[str, Any]]:
        """Decode JSON format"""
        try:
            # Parse header
            magic, payload_len, reserved = struct.unpack('<BHB', data[:self.HEADER_SIZE])
            
            if len(data) < self.HEADER_SIZE + payload_len:
                self._log(f"Incomplete JSON message: expected {payload_len}, got {len(data) - self.HEADER_SIZE}")
                return None
            
            # Extract payload
            payload = data[self.HEADER_SIZE:self.HEADER_SIZE + payload_len]
            
            # Deserialize
            json_str = payload.decode('utf-8')
            result = json.loads(json_str)
            self._log(f"Decoded JSON: {payload_len} bytes")
            
            return result
            
        except Exception as e:
            self._log(f"JSON decoding failed: {e}")
            return None
    
    def _decode_legacy(self, data: bytes) -> Optional[Dict[str, Any]]:
        """Decode legacy delimiter-based JSON format"""
        try:
            # Find delimiters
            if not data.startswith(self.LEGACY_START):
                # Try to find start delimiter
                start_idx = data.find(self.LEGACY_START)
                if start_idx < 0:
                    return None
                data = data[start_idx:]
            
            end_idx = data.find(self.LEGACY_END)
            if end_idx < 0:
                self._log("Legacy format: missing end delimiter")
                return None
            
            # Extract JSON payload
            json_bytes = data[len(self.LEGACY_START):end_idx]
            json_str = json_bytes.decode('utf-8')
            
            result = json.loads(json_str)
            self._log(f"Decoded legacy JSON: {len(json_bytes)} bytes")
            
            return result
            
        except Exception as e:
            self._log(f"Legacy JSON decoding failed: {e}")
            return None
    
    def detect_format(self, data: bytes) -> Optional[MessageFormat]:
        """
        Detect message format from raw bytes.
        
        Returns:
            MessageFormat enum or None if format cannot be determined
        """
        if len(data) < 1:
            return None
        
        if data.startswith(self.LEGACY_START):
            return MessageFormat.JSON  # Legacy is still JSON
        
        magic_byte = data[0]
        
        if magic_byte == MessageFormat.MSGPACK:
            return MessageFormat.MSGPACK
        elif magic_byte == MessageFormat.JSON:
            return MessageFormat.JSON
        else:
            return None
    
    def get_message_info(self, data: bytes) -> Dict[str, Any]:
        """
        Extract message metadata without full parsing.
        
        Returns:
            Dictionary with format, length, and validity information
        """
        info = {
            'valid': False,
            'format': None,
            'payload_length': 0,
            'total_length': 0,
            'complete': False
        }
        
        if len(data) < self.HEADER_SIZE:
            return info
        
        # Detect format
        fmt = self.detect_format(data)
        info['format'] = fmt.name if fmt else 'UNKNOWN'
        
        # Parse header for new formats
        if fmt in (MessageFormat.JSON, MessageFormat.MSGPACK):
            try:
                magic, payload_len, reserved = struct.unpack('<BHB', data[:self.HEADER_SIZE])
                info['valid'] = True
                info['payload_length'] = payload_len
                info['total_length'] = self.HEADER_SIZE + payload_len
                info['complete'] = len(data) >= info['total_length']
            except:
                pass
        
        # Legacy format
        elif fmt == MessageFormat.JSON and data.startswith(self.LEGACY_START):
            end_idx = data.find(self.LEGACY_END)
            if end_idx >= 0:
                info['valid'] = True
                info['complete'] = True
                info['payload_length'] = end_idx - len(self.LEGACY_START)
                info['total_length'] = end_idx + len(self.LEGACY_END)
        
        return info


# Convenience functions for backward compatibility
def encode_message(data: Dict[str, Any], use_msgpack: bool = True) -> bytes:
    """Convenience function to encode a message"""
    protocol = SerialProtocol(prefer_msgpack=use_msgpack)
    return protocol.encode(data)


def decode_message(data: bytes) -> Optional[Dict[str, Any]]:
    """Convenience function to decode a message"""
    protocol = SerialProtocol()
    return protocol.decode(data)


if __name__ == "__main__":
    # Test the protocol implementation
    print("=== Serial Protocol V2 Test ===\n")
    
    protocol = SerialProtocol(debug=True)
    
    # Test data
    test_command = {
        "task": "/motor_act",
        "qid": 42,
        "motor": {
            "steppers": [{
                "stepperid": 1,
                "position": 1000,
                "speed": 15000,
                "isabs": 0
            }]
        }
    }
    
    print("\n--- Testing MessagePack Encoding ---")
    if HAS_MSGPACK:
        encoded_mp = protocol.encode(test_command)
        print(f"Encoded size: {len(encoded_mp)} bytes")
        print(f"Hex: {encoded_mp[:20].hex()}...")
        
        decoded_mp = protocol.decode(encoded_mp)
        print(f"Decoded successfully: {decoded_mp == test_command}")
        print(f"QID match: {decoded_mp.get('qid') == 42}")
    else:
        print("MessagePack not available")
    
    print("\n--- Testing JSON Encoding ---")
    encoded_json = protocol.encode(test_command, force_json=True)
    print(f"Encoded size: {len(encoded_json)} bytes")
    print(f"Hex: {encoded_json[:20].hex()}...")
    
    decoded_json = protocol.decode(encoded_json)
    print(f"Decoded successfully: {decoded_json == test_command}")
    print(f"QID match: {decoded_json.get('qid') == 42}")
    
    print("\n--- Testing Legacy Format ---")
    encoded_legacy = protocol.encode_legacy(test_command)
    print(f"Encoded size: {len(encoded_legacy)} bytes")
    print(f"Sample: {encoded_legacy[:50]}...")
    
    decoded_legacy = protocol.decode(encoded_legacy)
    print(f"Decoded successfully: {decoded_legacy == test_command}")
    
    print("\n--- Size Comparison ---")
    if HAS_MSGPACK:
        mp_size = len(protocol.encode(test_command))
        json_size = len(protocol.encode(test_command, force_json=True))
        legacy_size = len(protocol.encode_legacy(test_command))
        
        print(f"MessagePack: {mp_size} bytes (100%)")
        print(f"JSON:        {json_size} bytes ({json_size/mp_size*100:.1f}%)")
        print(f"Legacy JSON: {legacy_size} bytes ({legacy_size/mp_size*100:.1f}%)")
        print(f"Savings:     {legacy_size - mp_size} bytes ({(1-mp_size/legacy_size)*100:.1f}% reduction)")
