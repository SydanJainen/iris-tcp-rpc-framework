"""Adapter: InMemoryTransactionLog — in-memory implementation of ITransactionLog."""

from typing import Optional
from uuid import UUID

from src.models import TransactionRecord


class InMemoryTransactionLog:
    """Stores TransactionRecord objects in a list; supports history and lookup."""

    def __init__(self) -> None:
        self._records: list[TransactionRecord] = []

    def log(self, record: TransactionRecord) -> None:
        self._records.append(record)

    def get_history(self, limit: int = 20) -> list[TransactionRecord]:
        if limit <= 0 or not self._records:
            return []
        return list(self._records[-limit:])

    def get_by_id(self, uuid: UUID) -> Optional[TransactionRecord]:
        for rec in self._records:
            if rec.id == uuid:
                return rec
        return None
