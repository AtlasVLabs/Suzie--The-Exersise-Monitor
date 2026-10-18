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

# Shared objects that the decoder updates and the display reads.
current_rep = Rep(0, 0, 0.0, 0.0, 0)
current_set = Set(0, 0, 0.0)

display_status = {"set": "Waiting"}