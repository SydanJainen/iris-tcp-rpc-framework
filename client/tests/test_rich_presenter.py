import uuid

import pytest

from rich.console import Console

from src.adapters.rich_presenter import RichPresenter
from src.models import MetricsSummary, TransactionRecord


def _make_presenter() -> tuple[RichPresenter, Console]:
    """Create a presenter with a captured console."""
    console = Console(file=None, force_terminal=True, width=120)
    presenter = RichPresenter(console=console)
    return presenter, console


def _capture(presenter: RichPresenter, console: Console, action) -> str:
    """Capture console output from a presenter action."""
    with console.capture() as capture:
        action()
    return capture.get()


class TestShowResult:

    def test_show_result_contains_value(self) -> None:
        presenter, console = _make_presenter()
        output = _capture(presenter, console, lambda: presenter.show_result("add", 42, 1.5))
        assert "42" in output

    def test_show_result_contains_command(self) -> None:
        presenter, console = _make_presenter()
        output = _capture(presenter, console, lambda: presenter.show_result("add", 42, 1.5))
        assert "add" in output

    def test_show_result_contains_time(self) -> None:
        presenter, console = _make_presenter()
        output = _capture(presenter, console, lambda: presenter.show_result("add", 42, 1.5))
        assert "1.5" in output


class TestShowError:

    def test_show_error_contains_code(self) -> None:
        presenter, console = _make_presenter()
        output = _capture(
            presenter, console,
            lambda: presenter.show_error("add", "ARG_ERROR", "wrong args"),
        )
        assert "ARG_ERROR" in output

    def test_show_error_contains_message(self) -> None:
        presenter, console = _make_presenter()
        output = _capture(
            presenter, console,
            lambda: presenter.show_error("add", "ARG_ERROR", "wrong args"),
        )
        assert "wrong args" in output


class TestShowHistory:

    def test_show_history_table_columns(self) -> None:
        records = [
            TransactionRecord(
                id=uuid.uuid4(),
                timestamp="2026-03-25T10:00:00",
                command="add",
                args=[1, 2],
                status="ok",
                result=3,
                round_trip_ms=1.2,
            ),
        ]
        presenter, console = _make_presenter()
        output = _capture(presenter, console, lambda: presenter.show_history(records))

        assert "Command" in output
        assert "Result" in output
        assert "Status" in output
        assert "add" in output
        assert "3" in output

    def test_show_history_truncated_uuid(self) -> None:
        test_uuid = uuid.UUID("12345678-1234-1234-1234-123456789abc")
        records = [
            TransactionRecord(
                id=test_uuid,
                timestamp="2026-03-25T10:00:00",
                command="add",
                args=[1, 2],
                status="ok",
                result=3,
                round_trip_ms=1.2,
            ),
        ]
        presenter, console = _make_presenter()
        output = _capture(presenter, console, lambda: presenter.show_history(records))
        assert "12345678" in output

    def test_show_history_empty(self) -> None:
        presenter, console = _make_presenter()
        output = _capture(presenter, console, lambda: presenter.show_history([]))
        assert "Transaction History" in output


class TestShowMetrics:

    def test_show_metrics_displays_data(self) -> None:
        summary = [
            MetricsSummary(
                command="add",
                total_calls=10,
                avg_time_ms=2.5,
                total_errors=1,
            ),
        ]
        presenter, console = _make_presenter()
        output = _capture(presenter, console, lambda: presenter.show_metrics(summary))
        assert "add" in output
        assert "10" in output
        assert "2.5" in output
