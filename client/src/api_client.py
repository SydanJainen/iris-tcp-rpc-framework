"""ApiClient: dynamically builds RPC methods from the server spec."""

import time
import uuid
from collections import defaultdict
from typing import Any

from src.ports.i_framer import IFramer
from src.ports.i_presenter import IPresenter
from src.ports.i_serializer import ISerializer
from src.ports.i_transport import ITransport
from src.models import MetricsSummary, TransactionRecord


class ApiClient:

    def __init__(
        self,
        transport: ITransport,
        framer: IFramer,
        serializer: ISerializer,
        presenter: IPresenter,
        host: str = "localhost",
        port: int = 5555,
    ) -> None:
        self._transport = transport
        self._framer = framer
        self._serializer = serializer
        self._presenter = presenter
        self._host = host
        self._port = port
        self._spec: dict = {}
        self._history: list[TransactionRecord] = []
        self._metrics: dict[str, dict[str, Any]] = defaultdict(
            lambda: {"calls": 0, "total_ms": 0.0, "errors": 0}
        )

        self._connect_and_load_spec()

    def _connect_and_load_spec(self) -> None:
        self._transport.connect(self._host, self._port)
        self._spec = self._fetch_spec()
        self._bind_methods()
        self._presenter.show_welcome(self._spec)

    def _fetch_spec(self) -> dict:
        req_id = str(uuid.uuid4())
        request = {"id": req_id, "command": "get_spec", "args": []}
        raw = self._serializer.serialize(request)
        framed = self._framer.pack(raw)
        self._transport.send(framed)

        response_raw = self._transport.receive()
        payload = self._framer.unpack(response_raw)
        response = self._serializer.deserialize(payload)

        if response.get("id") != req_id:
            raise RuntimeError("Spec response id mismatch")

        if response.get("status") != "success":
            raise RuntimeError(
                f"Failed to fetch spec: {response.get('message', 'unknown error')}"
            )

        return response["result"]

    def _bind_methods(self) -> None:
        for fn in self._spec.get("functions", []):
            name = fn["name"]
            expected_args = fn.get("args", [])
            self._create_method(name, expected_args)

    def _create_method(self, name: str, expected_args: list[dict]) -> None:
        def method(*args: Any) -> Any:
            return self._call(name, list(args), expected_args)

        method.__name__ = name
        method.__doc__ = (
            f"{name}({', '.join(a['name'] + ':' + a['type'] for a in expected_args)})"
        )
        setattr(self, name, method)

    def _call(
        self,
        cmd: str,
        args: list[Any],
        expected_args: list[dict],
    ) -> Any:
        if len(args) != len(expected_args):
            raise TypeError(
                f"{cmd} expects {len(expected_args)} argument(s), "
                f"got {len(args)}"
            )

        req_id = str(uuid.uuid4())
        request = {"id": req_id, "command": cmd, "args": args}

        raw = self._serializer.serialize(request)
        framed = self._framer.pack(raw)

        start = time.perf_counter()
        self._transport.send(framed)
        response_raw = self._transport.receive()
        elapsed_ms = (time.perf_counter() - start) * 1000

        payload = self._framer.unpack(response_raw)
        response = self._serializer.deserialize(payload)

        if response.get("id") != req_id:
            raise RuntimeError(
                f"Response id mismatch: expected {req_id}, "
                f"got {response.get('id')}"
            )

        record = TransactionRecord(
            id=uuid.UUID(req_id),
            timestamp=time.strftime("%Y-%m-%dT%H:%M:%S"),
            command=cmd,
            args=args,
            status=response.get("status", ""),
            result=response.get("result"),
            error_code=response.get("error_code"),
            message=response.get("message"),
            round_trip_ms=elapsed_ms,
        )
        self._history.append(record)

        self._metrics[cmd]["calls"] += 1
        self._metrics[cmd]["total_ms"] += elapsed_ms

        if response.get("status") == "success":
            return response["result"]

        self._metrics[cmd]["errors"] += 1
        error_code = response.get("error_code", "UNKNOWN")
        message = response.get("message", "Unknown error")
        raise RuntimeError(f"[{error_code}] {message}")

    def get_spec(self) -> dict:
        return self._spec

    def get_history(self) -> list[TransactionRecord]:
        return list(self._history)

    def get_metrics(self) -> list[MetricsSummary]:
        result: list[MetricsSummary] = []
        for cmd, data in self._metrics.items():
            avg_ms = data["total_ms"] / data["calls"] if data["calls"] else 0.0
            result.append(
                MetricsSummary(
                    command=cmd,
                    total_calls=data["calls"],
                    avg_time_ms=avg_ms,
                    total_errors=data["errors"],
                )
            )
        return result

    def close(self) -> None:
        self._transport.close()
