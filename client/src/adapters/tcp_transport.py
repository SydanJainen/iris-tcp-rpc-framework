"""Adapter: TCP socket transport."""

import socket
import struct


class TcpTransport:

    def __init__(self, timeout: float = 2.0) -> None:
        self._timeout = timeout
        self._sock: socket.socket | None = None

    def connect(self, host: str, port: int) -> None:
        try:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._sock.settimeout(self._timeout)
            self._sock.connect((host, port))
        except socket.timeout as exc:
            self._sock = None
            raise TimeoutError(
                f"Connection to {host}:{port} timed out"
            ) from exc
        except OSError as exc:
            self._sock = None
            raise ConnectionError(
                f"Failed to connect to {host}:{port}: {exc}"
            ) from exc

    def send(self, data: bytes) -> None:
        if self._sock is None:
            raise ConnectionError("Not connected")
        try:
            self._sock.sendall(data)
        except socket.timeout as exc:
            raise TimeoutError("Send timed out") from exc
        except OSError as exc:
            raise ConnectionError(f"Send failed: {exc}") from exc

    def receive(self) -> bytes:
        if self._sock is None:
            raise ConnectionError("Not connected")
        try:
            header = self._recv_exact(4)
            (length,) = struct.unpack("!I", header)
            payload = self._recv_exact(length)
            return header + payload
        except socket.timeout as exc:
            raise TimeoutError("Receive timed out") from exc
        except OSError as exc:
            raise ConnectionError(f"Receive failed: {exc}") from exc

    def close(self) -> None:
        if self._sock is not None:
            try:
                self._sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            try:
                self._sock.close()
            except OSError:
                pass
            finally:
                self._sock = None

    def _recv_exact(self, n: int) -> bytes:
        """Receive exactly n bytes from the socket."""
        chunks: list[bytes] = []
        remaining = n
        while remaining > 0:
            chunk = self._sock.recv(remaining)
            if not chunk:
                raise ConnectionError(
                    "Connection closed by remote host"
                )
            chunks.append(chunk)
            remaining -= len(chunk)
        return b"".join(chunks)
