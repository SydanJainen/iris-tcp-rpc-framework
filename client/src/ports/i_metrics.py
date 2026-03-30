"""Port: IMetrics — protocol for recording and querying performance metrics."""

from typing import Protocol

from src.models import MetricsSummary


class IMetrics(Protocol):

    def record(self, cmd: str, duration_ms: float, success: bool) -> None:
        ...

    def get_summary(self) -> list[MetricsSummary]:
        ...
