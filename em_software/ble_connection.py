"""BLE discovery, connection, and notification subscription."""

import asyncio

from bleak import BleakClient, BleakScanner

from packets import handle_packet

DEVICE_NAME = "Suzie"
TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"


def on_receive(characteristic, data: bytearray) -> None:
    handle_packet(data)


async def run_connection() -> None:
    print(f"Searching for {DEVICE_NAME}...")

    device = await BleakScanner.find_device_by_name(DEVICE_NAME, timeout=15.0)

    if device is None:
        print("Device not found.")
        return

    async with BleakClient(device) as client:
        print("Connected. Press Ctrl+C to disconnect.")
        await client.start_notify(TX_UUID, on_receive)

        while client.is_connected:
            await asyncio.sleep(0.5)

    print("Disconnected.")
