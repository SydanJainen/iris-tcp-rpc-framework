"""Unit tests for the JsonSerializer adapter."""

import json

import pytest

from src.adapters.json_serializer import JsonSerializer


@pytest.fixture
def serializer() -> JsonSerializer:
    return JsonSerializer()


# ---------- round-trip ----------


def test_round_trip_simple_object(serializer: JsonSerializer) -> None:
    obj = {"cmd": "add", "args": [10, 32]}
    result = serializer.deserialize(serializer.serialize(obj))
    assert result == obj


def test_round_trip_integer(serializer: JsonSerializer) -> None:
    obj = 42
    result = serializer.deserialize(serializer.serialize(obj))
    assert result == obj


def test_round_trip_string(serializer: JsonSerializer) -> None:
    obj = "hello world"
    result = serializer.deserialize(serializer.serialize(obj))
    assert result == obj


def test_round_trip_array(serializer: JsonSerializer) -> None:
    obj = [1, "two", 3.0, True, None]
    result = serializer.deserialize(serializer.serialize(obj))
    assert result == obj


# ---------- nested objects ----------


def test_round_trip_nested_object(serializer: JsonSerializer) -> None:
    obj = {
        "id": "abc-123",
        "status": "ok",
        "result": {
            "values": [1, 2, 3],
            "metadata": {"source": "test"},
        },
    }
    result = serializer.deserialize(serializer.serialize(obj))
    assert result == obj


# ---------- unicode ----------


def test_round_trip_unicode(serializer: JsonSerializer) -> None:
    obj = {
        "greeting": "こんにちは",
        "emoji": "🚀🌍",
        "accent": "café résumé",
    }
    result = serializer.deserialize(serializer.serialize(obj))
    assert result == obj


# ---------- empty structures ----------


def test_round_trip_empty_object(serializer: JsonSerializer) -> None:
    obj: dict = {}
    result = serializer.deserialize(serializer.serialize(obj))
    assert result == obj
    assert isinstance(result, dict)


def test_round_trip_empty_array(serializer: JsonSerializer) -> None:
    obj: list = []
    result = serializer.deserialize(serializer.serialize(obj))
    assert result == obj
    assert isinstance(result, list)


# ---------- error case ----------


def test_deserialize_invalid_json(serializer: JsonSerializer) -> None:
    with pytest.raises(json.JSONDecodeError):
        serializer.deserialize(b"not json")
