from dataclasses import dataclass, field
from typing import Any, Optional
from uuid import UUID

@dataclass
class TransactionRecord:

    id: UUID
    timestamp: str
    command: str
    args: list[Any] = field(default_factory=list)
    status: str = ""
    result: Any = None
    error_code: Optional[str] = None
    message: Optional[str] = None
    round_trip_ms: float = 0.0

@dataclass
class MetricsSummary:

    command: str = ""
    total_calls: int = 0
    avg_time_ms: float = 0.0
    total_errors: int = 0
