# Suzie

A wearable workout-monitoring prototype built with an ESP32 and ESP-IDF. Suzie processes sensor data to detect repetitions and track movement throughout a set.

## Current Features

- Phase-transition detection and repetition counting
- Automatic set detection with an inactivity timeout
- Rep and set duration tracking
- Per-rep range of motion and movement-effort metrics
- Set-level averages and exponential moving averages (EMA)
- Real-time debug output for signal and metric validation

Movement effort is a signal-derived metric, not a measurement of calories or mechanical work.

## Development

The project combines embedded C firmware, sensor integration, signal processing, and hardware prototyping. Rep and set metrics have been tested through live signal plots; broader workout validation is ongoing.

## Next Steps

- Transfer structured rep and set data to a host application
- Build a simple interactive terminal dashboard
- Measure detection accuracy and repeatability
- Develop a custom PCB and document board bring-up

**Status:** Active development.

---

<sub>This repository is shared for private review only. Please do not copy, redistribute, or reuse the code without permission.</sub>
