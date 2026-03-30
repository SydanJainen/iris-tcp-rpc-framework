"""REPL: interactive command loop for the Iris RPC client."""

from typing import Any

from src.api_client import ApiClient
from src.ports.i_presenter import IPresenter


BUILTIN_COMMANDS = {"help", "history", "metrics", "quit", "exit"}


def coerce_arg(value: str, type_hint: str) -> Any:
    """Coerce a string argument to the type specified in the spec."""
    if type_hint == "int":
        return int(value)
    elif type_hint == "double":
        return float(value)
    else:
        return value


def parse_input(line: str) -> tuple[str, list[str]]:
    """Split user input into command and argument tokens."""
    parts = line.strip().split()
    if not parts:
        return "", []
    return parts[0], parts[1:]


def coerce_args(raw_args: list[str], spec_args: list[dict]) -> list[Any]:
    """Coerce raw string arguments using the spec type hints."""
    result: list[Any] = []
    for i, raw in enumerate(raw_args):
        if i < len(spec_args):
            type_hint = spec_args[i].get("type", "string")
        else:
            type_hint = "string"
        result.append(coerce_arg(raw, type_hint))
    return result


def find_function_spec(spec: dict, cmd: str) -> dict | None:
    """Find the function spec entry for a given command name."""
    for fn in spec.get("functions", []):
        if fn["name"] == cmd:
            return fn
    return None


def run_repl(client: ApiClient, presenter: IPresenter) -> None:
    """Run the interactive REPL loop."""
    spec = client.get_spec()

    while True:
        try:
            presenter.show_prompt()
            line = input()
        except (EOFError, KeyboardInterrupt):
            presenter.show_error("system", "EXIT", "Interrupted")
            client.close()
            break

        cmd, raw_args = parse_input(line)

        if not cmd:
            continue

        if cmd in ("quit", "exit"):
            client.close()
            break

        if cmd == "help":
            presenter.show_welcome(spec)
            continue

        if cmd == "history":
            presenter.show_history(client.get_history())
            continue

        if cmd == "metrics":
            presenter.show_metrics(client.get_metrics())
            continue

        fn_spec = find_function_spec(spec, cmd)
        if fn_spec is None:
            presenter.show_error(cmd, "UNKNOWN_CMD", f"Unknown command: {cmd}")
            continue

        try:
            coerced = coerce_args(raw_args, fn_spec.get("args", []))
            method = getattr(client, cmd)
            result = method(*coerced)
            record = client.get_history()[-1]
            presenter.show_result(cmd, result, record.round_trip_ms)
        except TypeError as exc:
            presenter.show_error(cmd, "ARG_ERROR", str(exc))
        except ValueError as exc:
            presenter.show_error(cmd, "ARG_ERROR", f"Invalid argument: {exc}")
        except TimeoutError:
            presenter.show_error(cmd, "TIMEOUT", "Request timed out")
        except ConnectionError as exc:
            presenter.show_error(cmd, "CONN_ERROR", str(exc))
            client.close()
            break
        except RuntimeError as exc:
            presenter.show_error(cmd, "RPC_ERROR", str(exc))
