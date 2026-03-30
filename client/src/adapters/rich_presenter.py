"""Adapter: Rich-based console presenter."""

from typing import Any

from rich.console import Console
from rich.panel import Panel
from rich.table import Table

from src.models import MetricsSummary, TransactionRecord


class RichPresenter:

    def __init__(self, console: Console | None = None) -> None:
        self._console = console or Console()

    def show_result(self, cmd: str, result: Any, rt_ms: float) -> None:
        text = f"[bold]{result}[/bold]  [dim]({rt_ms:.1f} ms)[/dim]"
        panel = Panel(text, title=cmd, border_style="green", expand=False)
        self._console.print(panel)

    def show_error(self, cmd: str, error_code: str, message: str) -> None:
        self._console.print(
            f"[bold red]Error[/bold red] [{error_code}] {cmd}: {message}"
        )

    def show_history(self, records: list[TransactionRecord]) -> None:
        table = Table(title="Transaction History")
        table.add_column("ID", style="dim", width=8)
        table.add_column("Command")
        table.add_column("Result")
        table.add_column("Status")
        table.add_column("RT (ms)", justify="right")

        for rec in records:
            uuid_short = str(rec.id)[:8]
            status_style = "green" if rec.status == "ok" else "red"
            result_text = str(rec.result) if rec.result is not None else ""
            if rec.status != "ok" and rec.message:
                result_text = rec.message
            table.add_row(
                uuid_short,
                rec.command,
                result_text,
                f"[{status_style}]{rec.status}[/{status_style}]",
                f"{rec.round_trip_ms:.1f}",
            )

        self._console.print(table)

    def show_metrics(self, summary: list[MetricsSummary]) -> None:
        table = Table(title="Metrics")
        table.add_column("Command")
        table.add_column("Calls", justify="right")
        table.add_column("Avg RT (ms)", justify="right")
        table.add_column("Errors", justify="right")

        for m in summary:
            table.add_row(
                m.command,
                str(m.total_calls),
                f"{m.avg_time_ms:.1f}",
                str(m.total_errors),
            )

        self._console.print(table)

    def show_prompt(self) -> None:
        self._console.print("> ", end="")

    def show_welcome(self, spec: dict) -> None:
        self._console.print(
            Panel(
                "[bold]Iris RPC Client[/bold]",
                border_style="blue",
                expand=False,
            )
        )

        table = Table(title="Available Commands")
        table.add_column("Command")
        table.add_column("Arguments")
        table.add_column("Returns")

        for fn in spec.get("functions", []):
            args_str = ", ".join(
                f"{a['name']}:{a['type']}" for a in fn.get("args", [])
            )
            table.add_row(fn["name"], args_str, fn.get("returns", ""))

        self._console.print(table)
        self._console.print()
        self._console.print(
            "[dim]Commands: help, history, metrics, quit[/dim]"
        )
        self._console.print()
