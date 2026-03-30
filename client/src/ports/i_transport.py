from typing import Protocol


class ITransport(Protocol):

    def connect(self, host: str, port: int) -> None:
        ...

    def send(self, data: bytes) -> None:
        ...

    def receive(self) -> bytes:
        ...

    def close(self) -> None:
        ...
