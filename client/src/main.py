import argparse
import os
import sys

from src.adapters.in_memory_transaction_log import InMemoryTransactionLog
from src.adapters.json_serializer import JsonSerializer
from src.adapters.length_prefixed_framer import LengthPrefixedFramer
from src.adapters.metrics_collector import MetricsCollector
from src.adapters.rich_presenter import RichPresenter
from src.adapters.tcp_transport import TcpTransport
from src.api_client import ApiClient
from src.container import Container
from src.repl import run_repl


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Iris RPC Client",
    )
    parser.add_argument(
        "--host",
        default=os.environ.get("SERVER_HOST", "localhost"),
        help="Server host (default: localhost, env: SERVER_HOST)",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=int(os.environ.get("SERVER_PORT", "5555")),
        help="Server port (default: 5555, env: SERVER_PORT)",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=2.0,
        help="Socket timeout in seconds. default: 2.0",
    )
    return parser.parse_args(argv)


def build_container(args: argparse.Namespace) -> Container:
    container = Container()

    container.register("transport", lambda: TcpTransport(timeout=args.timeout), singleton=True)
    container.register("framer", lambda: LengthPrefixedFramer(), singleton=True)
    container.register("serializer", lambda: JsonSerializer(), singleton=True)
    container.register("presenter", lambda: RichPresenter(), singleton=True)
    container.register("metrics", lambda: MetricsCollector(), singleton=True)
    container.register("transaction_log", lambda: InMemoryTransactionLog(), singleton=True)
    container.register(
        "api_client",
        lambda: ApiClient(
            transport=container.resolve("transport"),
            framer=container.resolve("framer"),
            serializer=container.resolve("serializer"),
            presenter=container.resolve("presenter"),
            metrics=container.resolve("metrics"),
            transaction_log=container.resolve("transaction_log"),
            host=args.host,
            port=args.port,
        ),
        singleton=True,
    )

    return container


def main(argv: list[str] | None = None) -> None:
    args = parse_args(argv)

    try:
        container = build_container(args)
        client = container.resolve("api_client")
        presenter = container.resolve("presenter")
        run_repl(client, presenter)
    except (ConnectionError, TimeoutError) as exc:
        print(f"Connection failed: {exc}", file=sys.stderr)
        sys.exit(1)
    except KeyboardInterrupt:
        print("\nShutdown.")
        sys.exit(0)


if __name__ == "__main__":
    main()
