"""Unit tests for the ApiClient."""

import json
import struct
import uuid

import pytest

from src.adapters.json_serializer import JsonSerializer
from src.adapters.length_prefixed_framer import LengthPrefixedFramer
from src.api_client import ApiClient
from src.models import TransactionRecord


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


class MockMetrics:
    """A mock IMetrics that records all calls."""

    def __init__(self) -> None:
        self.calls: list[tuple[str, float, bool]] = []

    def record(self, cmd: str, duration_ms: float, success: bool) -> None:
        self.calls.append((cmd, duration_ms, success))

    def get_summary(self):
        return []


class MockTransactionLog:
    """A mock ITransactionLog that records all logged records."""

    def __init__(self) -> None:
        self.logged: list[TransactionRecord] = []

    def log(self, record: TransactionRecord) -> None:
        self.logged.append(record)

    def get_history(self, limit: int = 20) -> list[TransactionRecord]:
        return list(self.logged[-limit:])

    def get_by_id(self, uuid):
        for r in self.logged:
            if r.id == uuid:
                return r
        return None


def _build_client(
    extra_responses: list[dict] | None = None,
    metrics: MockMetrics | None = None,
    transaction_log: MockTransactionLog | None = None,
) -> tuple[ApiClient, MockTransport, MockMetrics, MockTransactionLog]:
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
    m = metrics if metrics is not None else MockMetrics()
    tl = transaction_log if transaction_log is not None else MockTransactionLog()

    client = ApiClient(
        transport=transport,
        framer=framer,
        serializer=serializer,
        presenter=presenter,
        metrics=m,
        transaction_log=tl,
        host="localhost",
        port=5555,
    )
    return client, transport, m, tl


class TestApiClientMethodGeneration:

    def test_spec_creates_bound_methods(self) -> None:
        client, _, _, _ = _build_client()
        assert hasattr(client, "add")
        assert hasattr(client, "reverse")
        assert callable(client.add)
        assert callable(client.reverse)

    def test_generated_method_produces_correct_request(self) -> None:
        add_resp = {"status": "ok", "result": 42}
        client, transport, _, _ = _build_client([add_resp])

        result = client.add(10, 32)
        assert result == 42

        req = transport.get_last_sent_request()
        assert req["cmd"] == "add"
        assert req["args"] == [10, 32]
        assert "id" in req


class TestApiClientResponseHandling:

    def test_matching_id_returns_result(self) -> None:
        resp = {"status": "ok", "result": 99}
        client, _, _, _ = _build_client([resp])
        assert client.add(1, 2) == 99

    def test_error_response_raises_runtime_error(self) -> None:
        resp = {
            "status": "error",
            "error_code": "UNKNOWN_CMD",
            "message": "Unknown command: foo",
        }
        client, _, _, _ = _build_client([resp])

        with pytest.raises(RuntimeError, match="UNKNOWN_CMD"):
            client.add(1, 2)


class TestApiClientArgValidation:

    def test_wrong_arg_count_raises_type_error(self) -> None:
        client, _, _, _ = _build_client()
        with pytest.raises(TypeError, match="expects 2"):
            client.add(1)

    def test_too_many_args_raises_type_error(self) -> None:
        client, _, _, _ = _build_client()
        with pytest.raises(TypeError, match="expects 2"):
            client.add(1, 2, 3)


class TestApiClientTimeout:

    def test_timeout_propagates(self) -> None:
        client, transport, _, _ = _build_client()

        def timeout_receive() -> bytes:
            raise TimeoutError("Receive timed out")

        transport.receive = timeout_receive

        with pytest.raises(TimeoutError):
            client.add(1, 2)


class TestApiClientMetricsIntegration:
    """Verify that metrics and transaction log are called after each request."""

    def test_metrics_record_called_on_success(self) -> None:
        resp = {"status": "ok", "result": 42}
        metrics = MockMetrics()
        client, _, m, _ = _build_client([resp], metrics=metrics)

        client.add(10, 32)

        assert len(m.calls) == 1
        cmd, duration_ms, success = m.calls[0]
        assert cmd == "add"
        assert duration_ms >= 0.0
        assert success is True

    def test_metrics_record_called_on_error(self) -> None:
        resp = {
            "status": "error",
            "error_code": "INTERNAL_ERROR",
            "message": "something failed",
        }
        metrics = MockMetrics()
        client, _, m, _ = _build_client([resp], metrics=metrics)

        with pytest.raises(RuntimeError):
            client.add(1, 2)

        assert len(m.calls) == 1
        cmd, duration_ms, success = m.calls[0]
        assert cmd == "add"
        assert success is False

    def test_transaction_log_called_on_success(self) -> None:
        resp = {"status": "ok", "result": 7}
        tl = MockTransactionLog()
        client, _, _, log = _build_client([resp], transaction_log=tl)

        client.add(3, 4)

        assert len(log.logged) == 1
        record = log.logged[0]
        assert record.command == "add"
        assert record.status == "ok"
        assert record.result == 7
        assert record.round_trip_ms >= 0.0

    def test_transaction_log_called_on_error(self) -> None:
        resp = {
            "status": "error",
            "error_code": "BAD_ARGS",
            "message": "bad args",
        }
        tl = MockTransactionLog()
        client, _, _, log = _build_client([resp], transaction_log=tl)

        with pytest.raises(RuntimeError):
            client.add(1, 2)

        assert len(log.logged) == 1
        record = log.logged[0]
        assert record.command == "add"
        assert record.status == "error"
        assert record.error_code == "BAD_ARGS"

    def test_transaction_record_has_uuid(self) -> None:
        resp = {"status": "ok", "result": 0}
        tl = MockTransactionLog()
        client, _, _, log = _build_client([resp], transaction_log=tl)

        client.add(0, 0)

        assert len(log.logged) == 1
        record = log.logged[0]
        # id must be a valid UUID
        assert isinstance(record.id, uuid.UUID)

    def test_multiple_calls_each_logged(self) -> None:
        responses = [
            {"status": "ok", "result": 3},
            {"status": "ok", "result": "olleh"},
        ]
        tl = MockTransactionLog()
        metrics = MockMetrics()
        client, _, m, log = _build_client(responses, metrics=metrics, transaction_log=tl)

        client.add(1, 2)
        client.reverse("hello")

        assert len(log.logged) == 2
        assert len(m.calls) == 2
        assert log.logged[0].command == "add"
        assert log.logged[1].command == "reverse"
