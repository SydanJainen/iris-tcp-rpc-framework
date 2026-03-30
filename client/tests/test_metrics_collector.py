"""Unit tests for MetricsCollector."""

import pytest

from src.adapters.metrics_collector import MetricsCollector
from src.models import MetricsSummary


class TestMetricsCollectorEmpty:

    def test_empty_returns_empty_list(self) -> None:
        mc = MetricsCollector()
        assert mc.get_summary() == []


class TestMetricsCollectorSingleCommand:

    def test_single_success_record(self) -> None:
        mc = MetricsCollector()
        mc.record("add", 10.0, True)

        summary = mc.get_summary()
        assert len(summary) == 1
        s = summary[0]
        assert s.command == "add"
        assert s.total_calls == 1
        assert s.avg_time_ms == pytest.approx(10.0)
        assert s.total_errors == 0

    def test_single_failure_record(self) -> None:
        mc = MetricsCollector()
        mc.record("add", 5.0, False)

        summary = mc.get_summary()
        assert len(summary) == 1
        s = summary[0]
        assert s.total_calls == 1
        assert s.total_errors == 1

    def test_multiple_records_average(self) -> None:
        mc = MetricsCollector()
        mc.record("add", 10.0, True)
        mc.record("add", 20.0, True)
        mc.record("add", 30.0, True)

        summary = mc.get_summary()
        assert len(summary) == 1
        s = summary[0]
        assert s.total_calls == 3
        assert s.avg_time_ms == pytest.approx(20.0)
        assert s.total_errors == 0

    def test_mixed_success_and_error(self) -> None:
        mc = MetricsCollector()
        mc.record("reverse", 5.0, True)
        mc.record("reverse", 10.0, False)
        mc.record("reverse", 15.0, True)

        summary = mc.get_summary()
        assert len(summary) == 1
        s = summary[0]
        assert s.total_calls == 3
        assert s.total_errors == 1
        assert s.avg_time_ms == pytest.approx(10.0)


class TestMetricsCollectorMultipleCommands:

    def test_separate_commands_separate_entries(self) -> None:
        mc = MetricsCollector()
        mc.record("add", 10.0, True)
        mc.record("reverse", 20.0, True)

        summary = mc.get_summary()
        assert len(summary) == 2
        cmds = {s.command for s in summary}
        assert cmds == {"add", "reverse"}

    def test_per_command_averages_are_independent(self) -> None:
        mc = MetricsCollector()
        mc.record("add", 10.0, True)
        mc.record("add", 30.0, True)
        mc.record("reverse", 100.0, True)

        summary = {s.command: s for s in mc.get_summary()}
        assert summary["add"].avg_time_ms == pytest.approx(20.0)
        assert summary["reverse"].avg_time_ms == pytest.approx(100.0)

    def test_per_command_error_counts_are_independent(self) -> None:
        mc = MetricsCollector()
        mc.record("add", 1.0, False)
        mc.record("add", 1.0, False)
        mc.record("reverse", 1.0, True)

        summary = {s.command: s for s in mc.get_summary()}
        assert summary["add"].total_errors == 2
        assert summary["reverse"].total_errors == 0


class TestMetricsCollectorEdgeCases:

    def test_zero_duration(self) -> None:
        mc = MetricsCollector()
        mc.record("add", 0.0, True)

        summary = mc.get_summary()
        assert summary[0].avg_time_ms == pytest.approx(0.0)

    def test_large_number_of_records(self) -> None:
        mc = MetricsCollector()
        for i in range(1000):
            mc.record("fib", float(i), i % 2 == 0)

        summary = mc.get_summary()
        assert len(summary) == 1
        s = summary[0]
        assert s.total_calls == 1000
        assert s.total_errors == 500  # every other is False
