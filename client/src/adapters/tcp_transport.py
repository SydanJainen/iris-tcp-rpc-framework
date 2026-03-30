"""Adapter: TCP socket transport."""

import logging
import socket
import struct
import time

logger = logging.getLogger(__name__)


class TcpTransport:

    MAX_RETRIES = 5
    RETRY_DELAY = 1.0  # seconds

    def __init__(self, timeout: float = 2.0) -> None:
        self._timeout = timeout
        self._sock: socket.socket | None = None

    def connect(self, host: str, port: int) -> None:
        last_error: Exception | None = None

        for attempt in range(1, self.MAX_RETRIES + 1):
            try:
                self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self._sock.settimeout(self._timeout)
                self._sock.connect((host, port))
                if attempt > 1:
                    logger.info(
                        "Connected to %s:%d on attempt %d", host, port, attempt
                    )
                return
            except socket.timeout as exc:
                self._sock = None
                last_error = TimeoutError(
                    f"Connection to {host}:{port} timed out"
                )
                last_error.__cause__ = exc
            except OSError as exc:
                self._sock = None
                last_error = ConnectionError(
                    f"Failed to connect to {host}:{port}: {exc}"
                )
                last_error.__cause__ = exc

            if attempt < self.MAX_RETRIES:
                logger.warning(
                    "Connection attempt %d/%d to %s:%d failed, "
                    "retrying in %.1fs...",
                    attempt, self.MAX_RETRIES, host, port, self.RETRY_DELAY,
                )
                time.sleep(self.RETRY_DELAY)

        raise last_error  # type: ignore[misc]

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
