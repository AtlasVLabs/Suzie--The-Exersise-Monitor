"""Decode Suzie's binary notifications and print their contents."""

import struct

from models import Rep, Set

PAYLOAD_SET_STARTED = 0xA001
PAYLOAD_REP_COMPLETED = 0xB001
PAYLOAD_SET_ENDED = 0xC001

ROM_SCALE = 1000.0
EFFORT_SCALE = 1000.0
SET_DURATION_SCALE = 1000.0

PACKET_LENGTHS = {
    PAYLOAD_SET_STARTED: 4,
    PAYLOAD_REP_COMPLETED: 12,
    PAYLOAD_SET_ENDED: 10,
}


def handle_packet(data: bytes | bytearray) -> None:
    if len(data) < 4:
        print("Packet is too small")
        return

    packet_id, command = struct.unpack_from("<HH", data)
    expected_length = PACKET_LENGTHS.get(command)

    if expected_length is None:
        print(f"Unknown command: 0x{command:04X}, packet {packet_id}")
        return

    if len(data) != expected_length:
        print(
            f"Incorrect packet length for 0x{command:04X}: "
            f"expected {expected_length}, received {len(data)}"
        )
        return

    match command:
        case 0xA001:
            print("Set started")

        case 0xB001:
            set_id, rep_id, rom, effort, duration_ms = struct.unpack_from(
                "<BBHHH", data, 4
            )
            rep = Rep(
                set_id=set_id,
                rep_id=rep_id,
                ROM=rom / ROM_SCALE,
                effort=effort / EFFORT_SCALE,
                duration_ms=duration_ms,
            )

            print("Rep Completed---------------")
            print(f"Rep ID: {rep.rep_id}")
            print(f"Rep ROM: {rep.ROM}")
            print(f"Rep Effort: {rep.effort}")
            print(f"Rep Duration: {rep.duration_ms}\n----------------------------")

        case 0xC001:
            set_id, rep_count, set_duration = struct.unpack_from("<BBI", data, 4)

            set = Set(
                set_id = set_id,
                rep_count = rep_count,
                set_duration = set_duration/SET_DURATION_SCALE,
            )
            print("Set ended")
            print(f"Set ID: {set.set_id}")
            print(f"Rep Count: {set.rep_count}")
            print(f"Set Duration: {set.set_duration}")
