import asyncio
from unittest import case
from bleak import BleakClient, BleakScanner
from dataclasses import dataclass
import struct

DEVICE_NAME = "Suzie"  # Change if you renamed the device

TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"



@dataclass
class Rep:
    set_id: int
    rep_id: int
    ROM: float
    effort: float
    duration_ms: int

def on_receive(characteristic, data):
    if len(data) < 4:
        print("Packet is too small")
        return
    p_ID = int.from_bytes(data[:2], byteorder="little")
    command = int.from_bytes(data[2:4], byteorder="little")

    match command:
        case 0xA001:
            print(f"Set started")
            # idk what goes here yet
            pass
        case 0xB001:
            print(f"Rep Completed---------------")
            set_id, rep_id, rom, effort, duration_ms = struct.unpack("<BBHHH", data[4:])
            print(f"Rep ID: {rep_id}")
            print(f"Rep ROM: {rom/1000.0}")
            print(f"Rep Effort: {effort/1000.0}")
            print(f"Rep Duration: {duration_ms}\n----------------------------")
            pass
        case 0xC001:
            print(f"Set ended")
            set_id, rep_count, set_duration = struct.unpack("<BBI", data[4:])
            print(f"Set ID: {set_id}")
            print(f"Rep Count: {rep_count}")
            print(f"Set Duration: {set_duration}")
            pass
        case _:
            pass


async def main():
    print(f"Searching for {DEVICE_NAME}...")

    device = await BleakScanner.find_device_by_name(
        DEVICE_NAME, timeout=15.0
    )

    if device is None:
        print("Device not found.")
        return

    async with BleakClient(device) as client:
        print("Connected. Press Ctrl+C to disconnect.")

        await client.start_notify(TX_UUID, on_receive)

        while client.is_connected:
            await asyncio.sleep(0.5)

    print("Disconnected.")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nStopped.")