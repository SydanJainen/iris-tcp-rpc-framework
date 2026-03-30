#!/usr/bin/env python3
"""Standalone integration test for the Iris RPC server.

Connects to the server, exercises core commands, and reports results.
Exit code 0 on success, 1 on any failure.

Usage:
    python tools/integration_test.py --host localhost --port 5555
"""

import argparse
import json
import math
import socket
import struct
import sys
import uuid


# ---------------------------------------------------------------------------
# Low-level helpers (mirrors the client protocol)
# ---------------------------------------------------------------------------

def _pack(payload: bytes) -> bytes:
    """Length-prefixed framing: 4-byte big-endian length + payload."""
    return struct.pack("!I", len(payload)) + payload


def _recv_exact(sock: socket.socket, n: int) -> bytes:
    """Receive exactly *n* bytes."""
    chunks: list[bytes] = []
    remaining = n
    while remaining > 0:
        chunk = sock.recv(remaining)
        if not chunk:
            raise ConnectionError("Connection closed by remote host")
        chunks.append(chunk)
        remaining -= len(chunk)
    return b"".join(chunks)


def _receive(sock: socket.socket) -> bytes:
    """Read one length-prefixed message."""
    header = _recv_exact(sock, 4)
    (length,) = struct.unpack("!I", header)
    return _recv_exact(sock, length)


def _call(sock: socket.socket, cmd: str, args: list) -> dict:
    """Send a JSON-RPC-style request and return the parsed response."""
    req_id = str(uuid.uuid4())
    request = {"id": req_id, "cmd": cmd, "args": args}
    raw = json.dumps(request).encode("utf-8")
    sock.sendall(_pack(raw))
    response_raw = _receive(sock)
    response = json.loads(response_raw.decode("utf-8"))
    if response.get("id") != req_id:
        raise RuntimeError(f"ID mismatch: expected {req_id}, got {response.get('id')}")
    return response


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

def test_get_spec(sock: socket.socket) -> None:
    resp = _call(sock, "get_spec", [])
    assert resp["status"] == "ok", f"get_spec failed: {resp}"
    spec = resp["result"]
    assert "functions" in spec, "Spec missing 'functions' key"
    names = {f["name"] for f in spec["functions"]}
    assert "add" in names, "Spec missing 'add' function"
    assert "reverse" in names, "Spec missing 'reverse' function"
    assert "multiply" in names, "Spec missing 'multiply' function"


def test_add(sock: socket.socket) -> None:
    resp = _call(sock, "add", [10, 32])
    assert resp["status"] == "ok", f"add failed: {resp}"
    assert resp["result"] == 42, f"add(10,32) expected 42, got {resp['result']}"


def test_reverse(sock: socket.socket) -> None:
    resp = _call(sock, "reverse", ["hello"])
    assert resp["status"] == "ok", f"reverse failed: {resp}"
    assert resp["result"] == "olleh", (
        f"reverse('hello') expected 'olleh', got {resp['result']}"
    )


def test_multiply(sock: socket.socket) -> None:
    resp = _call(sock, "multiply", [3.14, 2.0])
    assert resp["status"] == "ok", f"multiply failed: {resp}"
    assert math.isclose(resp["result"], 6.28, rel_tol=1e-6), (
        f"multiply(3.14,2.0) expected ~6.28, got {resp['result']}"
    )


def test_unknown_command(sock: socket.socket) -> None:
    resp = _call(sock, "nonexistent_command", [])
    assert resp["status"] == "error", (
        f"Unknown command should return error, got: {resp}"
    )


# ---------------------------------------------------------------------------
# Runner
# ---------------------------------------------------------------------------

ALL_TESTS = [
    ("get_spec", test_get_spec),
    ("add(10,32)=42", test_add),
    ("reverse('hello')='olleh'", test_reverse),
    ("multiply(3.14,2.0)~=6.28", test_multiply),
    ("unknown command error", test_unknown_command),
]


def main() -> None:
    parser = argparse.ArgumentParser(description="Iris RPC integration tests")
    parser.add_argument("--host", default="localhost", help="Server host")
    parser.add_argument("--port", type=int, default=5555, help="Server port")
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5.0)

    try:
        sock.connect((args.host, args.port))
    except OSError as exc:
        print(f"FAIL: cannot connect to {args.host}:{args.port}: {exc}")
        sys.exit(1)

    passed = 0
    failed = 0

    for name, test_fn in ALL_TESTS:
        try:
            test_fn(sock)
            print(f"  PASS  {name}")
            passed += 1
        except Exception as exc:
            print(f"  FAIL  {name}: {exc}")
            failed += 1

    sock.close()

    print(f"\n{passed} passed, {failed} failed, {passed + failed} total")
    sys.exit(1 if failed > 0 else 0)


if __name__ == "__main__":
    main()
