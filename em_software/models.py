"""Data objects used by the application."""

from dataclasses import dataclass


@dataclass
class Rep:
    set_id: int
    rep_id: int
    ROM: float
    effort: float
    duration_ms: int

@dataclass
class Set:
    set_id: int
    rep_count: int
    set_duration: float
