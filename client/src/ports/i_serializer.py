from typing import Protocol


class ISerializer(Protocol):

    def serialize(self, obj: dict) -> bytes:
        ...

    def deserialize(self, data: bytes) -> dict:
        ...
