"""Decode Suzie's binary notifications and print their contents."""

import struct

from models import Rep, current_rep, current_set, display_status
from models import rom_samples, effort_samples, duration_samples

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
            display_status["set"] = "Active"
            current_set.rep_count = 0
            current_set.set_duration = 0.0

            rom_samples.clear()
            effort_samples.clear()
            duration_samples.clear()

        case 0xB001:
            set_id, rep_id, rom, effort, duration_ms = struct.unpack_from(
                "<BBHHH", data, 4
            )

            current_rep.set_id = set_id
            current_rep.rep_id = rep_id
            current_rep.ROM = rom / ROM_SCALE
            current_rep.effort = effort
            current_rep.duration_ms = duration_ms

            current_set.set_id = set_id
            current_set.rep_count = rep_id
            display_status["set"] = "Active"

            rom_samples.append(current_rep.ROM)
            effort_samples.append(current_rep.effort)
            duration_samples.append(current_rep.duration_ms / 1000.0)

            # Keep the most recent 20 samples.
            del rom_samples[:-20]
            del effort_samples[:-20]
            del duration_samples[:-20]

        case 0xC001:
            set_id, rep_count, set_duration = struct.unpack_from("<BBI", data, 4)

            current_set.set_id = set_id
            current_set.rep_count = rep_count
            current_set.set_duration = set_duration / 1000.0
            display_status["set"] = "Complete / resting"
