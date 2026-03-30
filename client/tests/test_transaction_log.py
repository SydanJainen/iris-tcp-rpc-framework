"""Unit tests for InMemoryTransactionLog."""

import uuid
from datetime import datetime, timezone

import pytest

from src.adapters.in_memory_transaction_log import InMemoryTransactionLog
from src.models import TransactionRecord


def _make_record(cmd: str = "add", status: str = "ok") -> TransactionRecord:
    return TransactionRecord(
        id=uuid.uuid4(),
        timestamp=datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%S"),
        command=cmd,
        args=[1, 2],
        status=status,
        result=3 if status == "ok" else None,
        round_trip_ms=5.0,
    )


class TestInMemoryTransactionLogEmpty:

    def test_get_history_on_empty_returns_empty_list(self) -> None:
        log = InMemoryTransactionLog()
        assert log.get_history() == []

    def test_get_by_id_on_empty_returns_none(self) -> None:
        log = InMemoryTransactionLog()
        assert log.get_by_id(uuid.uuid4()) is None


class TestInMemoryTransactionLogLog:

    def test_log_single_record(self) -> None:
        log = InMemoryTransactionLog()
        rec = _make_record()
        log.log(rec)

        history = log.get_history()
        assert len(history) == 1
        assert history[0].id == rec.id

    def test_log_multiple_records_in_order(self) -> None:
        log = InMemoryTransactionLog()
        records = [_make_record(cmd=f"cmd_{i}") for i in range(5)]
        for r in records:
            log.log(r)

        history = log.get_history(limit=10)
        assert len(history) == 5
        for i, h in enumerate(history):
            assert h.id == records[i].id


class TestInMemoryTransactionLogGetHistory:

    def test_get_history_default_limit(self) -> None:
        log = InMemoryTransactionLog()
        for _ in range(25):
            log.log(_make_record())

        history = log.get_history()
        assert len(history) == 20  # default limit is 20

    def test_get_history_with_limit(self) -> None:
        log = InMemoryTransactionLog()
        for _ in range(10):
            log.log(_make_record())

        history = log.get_history(limit=3)
        assert len(history) == 3

    def test_get_history_returns_last_n(self) -> None:
        log = InMemoryTransactionLog()
        records = [_make_record(cmd=f"cmd_{i}") for i in range(5)]
        for r in records:
            log.log(r)

        history = log.get_history(limit=2)
        assert len(history) == 2
        assert history[0].id == records[3].id
        assert history[1].id == records[4].id

    def test_get_history_limit_larger_than_stored(self) -> None:
        log = InMemoryTransactionLog()
        for _ in range(3):
            log.log(_make_record())

        history = log.get_history(limit=100)
        assert len(history) == 3

    def test_get_history_zero_limit_returns_empty(self) -> None:
        log = InMemoryTransactionLog()
        log.log(_make_record())

        history = log.get_history(limit=0)
        assert history == []

    def test_get_history_negative_limit_returns_empty(self) -> None:
        log = InMemoryTransactionLog()
        log.log(_make_record())

        history = log.get_history(limit=-1)
        assert history == []


class TestInMemoryTransactionLogGetById:

    def test_get_by_id_finds_record(self) -> None:
        log = InMemoryTransactionLog()
        rec = _make_record()
        log.log(rec)

        found = log.get_by_id(rec.id)
        assert found is not None
        assert found.id == rec.id
        assert found.command == rec.command

    def test_get_by_id_returns_none_for_unknown(self) -> None:
        log = InMemoryTransactionLog()
        log.log(_make_record())

        found = log.get_by_id(uuid.uuid4())
        assert found is None

    def test_get_by_id_finds_correct_among_many(self) -> None:
        log = InMemoryTransactionLog()
        records = [_make_record(cmd=f"cmd_{i}") for i in range(10)]
        for r in records:
            log.log(r)

        target = records[5]
        found = log.get_by_id(target.id)
        assert found is not None
        assert found.command == target.command

    def test_get_by_id_all_unique_ids(self) -> None:
        log = InMemoryTransactionLog()
        records = [_make_record() for _ in range(5)]
        for r in records:
            log.log(r)

        for rec in records:
            found = log.get_by_id(rec.id)
            assert found is not None
            assert found.id == rec.id
