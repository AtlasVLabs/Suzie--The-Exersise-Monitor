import math

from rich.text import Text


def block_graph(
    samples,
    width=20,
    height=4,
    minimum=None,
    maximum=None
):
    if width < 1 or height < 1:
        raise ValueError("Width and height must be positive")

    # One column per sample; show the most recent ones.
    values = [float(value) for value in samples[-width:]]

    if not all(math.isfinite(value) for value in values):
        raise ValueError("Samples must be finite numbers")

    if not values:
        return Text("\n".join([" " * width] * height))

    low = min(values) if minimum is None else minimum
    high = max(values) if maximum is None else maximum

    # Add some space so the smallest sample is still visible.
    if high > low:
        padding = (high - low) * 0.1
    else:
        # Handles one sample or several identical samples.
        padding = max(abs(low) * 0.1, 0.1)

    if minimum is None:
        low -= padding

    if maximum is None:
        high += padding

    if high <= low:
        raise ValueError("Maximum must exceed minimum")

    # Each terminal row supports eight partial block heights.
    blocks = " ▁▂▃▄▅▆▇█"
    bar_heights = []

    for value in values:
        fraction = (value - low) / (high - low)
        fraction = max(0.0, min(1.0, fraction))

        bar_heights.append(round(fraction * height * 8))

    lines = []

    # Build the graph from its top row down.
    for row in range(height):
        line = ""

        for bar_height in bar_heights:
            remaining = bar_height - (height - 1 - row) * 8
            block_index = max(0, min(8, remaining))
            line += blocks[block_index]

        lines.append(line.ljust(width))

    return Text("\n".join(lines), style="cyan", no_wrap=True)