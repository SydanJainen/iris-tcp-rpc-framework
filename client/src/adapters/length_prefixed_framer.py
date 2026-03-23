"""Adapter: length-prefixed message framing."""

import struct


class LengthPrefixedFramer:

    def pack(self, payload: bytes) -> bytes:
        return struct.pack("!I", len(payload)) + payload

    def unpack(self, stream: bytes) -> bytes:

        if len(stream) < 4:
            raise ValueError(
                f"LengthPrefixedFramer.unpack: stream too short for header "
                f"(need 4 bytes, got {len(stream)})"
            )

        (length,) = struct.unpack("!I", stream[:4])

        if len(stream) < 4 + length:
            raise ValueError(
                f"LengthPrefixedFramer.unpack: stream too short for payload "
                f"(header has {length} bytes, but only "
                f"{len(stream) - 4} available)"
            )

        return stream[4 : 4 + length]
