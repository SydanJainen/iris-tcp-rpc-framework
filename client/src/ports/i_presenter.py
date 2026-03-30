from typing import Any, Protocol

from src.models import MetricsSummary, TransactionRecord


class IPresenter(Protocol):

    def show_result(self, cmd: str, result: Any, rt_ms: float) -> None:
        ...

    def show_error(self, cmd: str, error_code: str, message: str) -> None:
        ...

    def show_history(self, records: list[TransactionRecord]) -> None:
        ...

    def show_metrics(self, summary: list[MetricsSummary]) -> None:
        ...

    def show_prompt(self) -> None:
        ...

    def show_welcome(self, spec: dict) -> None:
        ...
