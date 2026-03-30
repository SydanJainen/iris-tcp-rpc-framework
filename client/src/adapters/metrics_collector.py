"""Adapter: MetricsCollector — in-memory implementation of IMetrics."""

from collections import defaultdict

from src.models import MetricsSummary


class MetricsCollector:
    """Accumulates per-command call counts, total time, and error counts.

    On the client side the duration recorded is the full round-trip time.
    """

    def __init__(self) -> None:
        self._data: dict[str, dict] = defaultdict(
            lambda: {"calls": 0, "total_ms": 0.0, "errors": 0}
        )

    def record(self, cmd: str, duration_ms: float, success: bool) -> None:
        entry = self._data[cmd]
        entry["calls"] += 1
        entry["total_ms"] += duration_ms
        if not success:
            entry["errors"] += 1

    def get_summary(self) -> list[MetricsSummary]:
        result: list[MetricsSummary] = []
        for cmd, data in self._data.items():
            calls = data["calls"]
            avg_ms = data["total_ms"] / calls if calls > 0 else 0.0
            result.append(
                MetricsSummary(
                    command=cmd,
                    total_calls=calls,
                    avg_time_ms=avg_ms,
                    total_errors=data["errors"],
                )
            )
        return result
