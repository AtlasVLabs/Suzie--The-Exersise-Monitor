from rich import box
from rich.console import Group
from rich.panel import Panel
from rich.table import Table
from rich.text import Text

from models import current_rep, current_set, display_status


def make_dashboard():
    # Header
    header = Table.grid(expand=True)
    header.add_column(ratio=1)
    header.add_column(justify="right")

    header.add_row("Suzie", "Connected")

    # Rep values
    rep_table = Table.grid(expand=True, padding=(0, 1))
    rep_table.add_column(style="cyan")
    rep_table.add_column(justify="right")

    rep_table.add_row("Set ID", str(current_rep.set_id))
    rep_table.add_row("Rep ID", str(current_rep.rep_id))
    rep_table.add_row("ROM", f"{current_rep.ROM:.3f} rad")
    rep_table.add_row("Effort", f"{current_rep.effort:.3f}")
    rep_table.add_row(
        "Duration",
        f"{current_rep.duration_ms / 1000:.2f} s"
    )

    # Three placeholders; each has four content rows plus two borders.
    graphs = Table.grid(expand=True, padding=(0, 1))

    for _ in range(3):
        graphs.add_column(ratio=1)

    graphs.add_row(
        Panel("", title="ROM", height=6, box=box.SQUARE),
        Panel("", title="Effort", height=6, box=box.SQUARE),
        Panel("", title="Duration", height=6, box=box.SQUARE),
    )

    rep_content = Group(
        rep_table,
        Text(""),
        Text("Rep history — graphs to be added", style="dim"),
        graphs,
    )

    # Set values
    set_table = Table.grid(expand=True, padding=(0, 1))
    set_table.add_column(style="cyan")
    set_table.add_column(justify="right")

    set_table.add_row("State", display_status["set"])
    set_table.add_row("Set ID", str(current_set.set_id))
    set_table.add_row("Reps", str(current_set.rep_count))

    duration = "—"
    if display_status["set"] == "Complete / resting":
        duration = f"{current_set.set_duration:.2f} s"

    set_table.add_row("Set duration", duration)

    return Group(
        Panel(header, box=box.SQUARE, border_style="cyan"),
        Panel(
            rep_content,
            title="Rep",
            box=box.SQUARE,
            border_style="cyan",
        ),
        Panel(
            set_table,
            title="Set",
            box=box.SQUARE,
            border_style="cyan",
        ),
    )