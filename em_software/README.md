# Suzie Python receiver

Requires Python 3.10 or newer. This version prints received metrics; it does not add Rich or Live yet.

Copy main.py, ble_connection.py, packets.py, and models.py into your existing em_software folder together. Back up your previous main.py before replacing it. Keep your existing .venv.

From the em_software terminal, run:

```powershell
.\.venv\Scripts\python.exe main.py
```

If Bleak is not installed:

```powershell
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
```

Disconnect your phone from Suzie before connecting from the computer. Set DEVICE_NAME in ble_connection.py to the advertised name if it differs from Suzie.

## Files

- main.py: starts the application and handles Ctrl+C.
- ble_connection.py: discovers the device, connects, and forwards notifications.
- packets.py: validates packet sizes, decodes little-endian fields, and prints metrics.
- models.py: defines the Rep dataclass.

Each BLE notification must contain one complete packet. ROM and effort assume firmware encoding with a multiplier of 1000. Durations are printed in milliseconds.

The original valid-packet output is preserved. Malformed lengths and unknown commands now print a diagnostic instead of failing in unpacking or being silently ignored. No sequence tracking, persistence, reconnection, or dashboard is added.
