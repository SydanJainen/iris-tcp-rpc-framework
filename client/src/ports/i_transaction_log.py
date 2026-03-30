"""Port: ITransactionLog — protocol for logging and querying transactions."""

from typing import Optional, Protocol
from uuid import UUID

from src.models import TransactionRecord


class ITransactionLog(Protocol):

    def log(self, record: TransactionRecord) -> None:
        ...

    def get_history(self, limit: int = 20) -> list[TransactionRecord]:
        ...

    def get_by_id(self, uuid: UUID) -> Optional[TransactionRecord]:
        ...
