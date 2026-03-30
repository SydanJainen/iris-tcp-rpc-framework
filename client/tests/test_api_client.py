"""Unit tests for the ApiClient."""

import json
import struct
import uuid

import pytest

from src.adapters.json_serializer import JsonSerializer
from src.adapters.length_prefixed_framer import LengthPrefixedFramer
from src.api_client import ApiClient


SAMPLE_SPEC = {
    "version": "1.0",
    "functions": [
        {
            "name": "add",
            "args": [
                {"name": "a", "type": "int"},
                {"name": "b", "type": "int"},
            ],
            "returns": "int",
        },
        {
            "name": "reverse",
            "args": [{"name": "s", "type": "string"}],
            "returns": "string",
        },
    ],
}


class MockTransport:
    """A mock transport that auto-echoes request ids in responses.

    Each queued response template has its "id" field replaced with the
    id from the corresponding sent request, so the ApiClient id-check
    always passes.
    """

    def __init__(self, response_templates: list[dict] | None = None) -> None:
        self._templates = list(response_templates) if response_templates else []
        self._sent: list[bytes] = []
        self._call_index = 0

    def connect(self, host: str, port: int) -> None:
        pass

    def send(self, data: bytes) -> None:
        self._sent.append(data)

    def receive(self) -> bytes:
        if self._call_index >= len(self._templates):
            raise ConnectionError("No more mock responses")

        # Decode the request that was just sent to extract its id
        raw = self._sent[-1]
        length = struct.unpack("!I", raw[:4])[0]
        req = json.loads(raw[4 : 4 + length].decode("utf-8"))

        template = dict(self._templates[self._call_index])
        template["id"] = req["id"]  # echo back the request id
        self._call_index += 1

        payload = json.dumps(template).encode("utf-8")
        return struct.pack("!I", len(payload)) + payload

    def close(self) -> None:
        pass

    def get_last_sent_request(self) -> dict:
        raw = self._sent[-1]
        length = struct.unpack("!I", raw[:4])[0]
        return json.loads(raw[4 : 4 + length].decode("utf-8"))


class MockPresenter:
    """A no-op presenter for tests."""

    def show_result(self, cmd, result, rt_ms):
        pass

    def show_error(self, cmd, error_code, message):
        pass

    def show_history(self, records):
        pass

    def show_metrics(self, summary):
        pass

    def show_prompt(self):
        pass

    def show_welcome(self, spec):
        pass


def _build_client(
    extra_responses: list[dict] | None = None,
) -> tuple[ApiClient, MockTransport]:
    """Build an ApiClient with a MockTransport.

    The first response is always the spec response.  Additional
    response templates can be passed via extra_responses; their "id"
    fields are automatically matched at receive time.
    """
    spec_resp = {"status": "ok", "result": SAMPLE_SPEC}
    all_responses = [spec_resp] + (extra_responses or [])

    transport = MockTransport(all_responses)
    framer = LengthPrefixedFramer()
    serializer = JsonSerializer()
    presenter = MockPresenter()

    client = ApiClient(
        transport=transport,
        framer=framer,
        serializer=serializer,
        presenter=presenter,
        host="localhost",
        port=5555,
    )
    return client, transport


class TestApiClientMethodGeneration:

    def test_spec_creates_bound_methods(self) -> None:
        client, _ = _build_client()
        assert hasattr(client, "add")
        assert hasattr(client, "reverse")
        assert callable(client.add)
        assert callable(client.reverse)

    def test_generated_method_produces_correct_request(self) -> None:
        add_resp = {"status": "ok", "result": 42}
        client, transport = _build_client([add_resp])

        result = client.add(10, 32)
        assert result == 42

        req = transport.get_last_sent_request()
        assert req["cmd"] == "add"
        assert req["args"] == [10, 32]
        assert "id" in req


class TestApiClientResponseHandling:

    def test_matching_id_returns_result(self) -> None:
        resp = {"status": "ok", "result": 99}
        client, _ = _build_client([resp])
        assert client.add(1, 2) == 99

    def test_error_response_raises_runtime_error(self) -> None:
        resp = {
            "status": "error",
            "error_code": "UNKNOWN_CMD",
            "message": "Unknown command: foo",
        }
        client, _ = _build_client([resp])

        with pytest.raises(RuntimeError, match="UNKNOWN_CMD"):
            client.add(1, 2)


class TestApiClientArgValidation:

    def test_wrong_arg_count_raises_type_error(self) -> None:
        client, _ = _build_client()
        with pytest.raises(TypeError, match="expects 2"):
            client.add(1)

    def test_too_many_args_raises_type_error(self) -> None:
        client, _ = _build_client()
        with pytest.raises(TypeError, match="expects 2"):
            client.add(1, 2, 3)


class TestApiClientTimeout:

    def test_timeout_propagates(self) -> None:
        client, transport = _build_client()

        def timeout_receive() -> bytes:
            raise TimeoutError("Receive timed out")

        transport.receive = timeout_receive

        with pytest.raises(TimeoutError):
            client.add(1, 2)
