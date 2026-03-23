from typing import Protocol


class IFramer(Protocol):

    def pack(self, payload: bytes) -> bytes:
        ...

    def unpack(self, stream: bytes) -> bytes:
        ...
