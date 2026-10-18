"""Application entry point. Run this file."""

import asyncio

from ble_connection import run_connection


if __name__ == "__main__":
    try:
        asyncio.run(run_connection())
    except KeyboardInterrupt:
        print("\nStopped.")
