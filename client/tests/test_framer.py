"""Unit tests for the LengthPrefixedFramer adapter."""

import struct

import pytest

from src.adapters.length_prefixed_framer import LengthPrefixedFramer


@pytest.fixture
def framer() -> LengthPrefixedFramer:
    return LengthPrefixedFramer()

def test_pack_unpack_symmetry_small(framer: LengthPrefixedFramer) -> None:
    payload = b"Hello"
    assert framer.unpack(framer.pack(payload)) == payload


def test_pack_unpack_symmetry_single_byte(framer: LengthPrefixedFramer) -> None:
    payload = b"\xff"
    assert framer.unpack(framer.pack(payload)) == payload


def test_pack_unpack_symmetry_medium(framer: LengthPrefixedFramer) -> None:
    payload = bytes(range(256))
    assert framer.unpack(framer.pack(payload)) == payload


# ---------- empty payload ----------


def test_empty_payload(framer: LengthPrefixedFramer) -> None:
    payload = b""
    packed = framer.pack(payload)

    # Frame should be exactly 4 bytes: 00 00 00 00
    assert len(packed) == 4
    assert packed == b"\x00\x00\x00\x00"

    assert framer.unpack(packed) == b""


# ---------- large payload ----------


def test_large_payload(framer: LengthPrefixedFramer) -> None:
    payload = b"\xab" * 100_000
    packed = framer.pack(payload)
    assert len(packed) == 4 + len(payload)
    assert framer.unpack(packed) == payload


# ---------- header encoding ----------


def test_header_is_big_endian(framer: LengthPrefixedFramer) -> None:
    payload = b"\x00" * 256
    packed = framer.pack(payload)
    assert packed[:4] == b"\x00\x00\x01\x00"


# ---------- error cases ----------


def test_unpack_too_short_for_header(framer: LengthPrefixedFramer) -> None:
    with pytest.raises(ValueError, match="too short for header"):
        framer.unpack(b"\x00\x00")


def test_unpack_too_short_for_payload(framer: LengthPrefixedFramer) -> None:
    stream = struct.pack("!I", 10) + b"\x01\x02"
    with pytest.raises(ValueError, match="too short for payload"):
        framer.unpack(stream)
