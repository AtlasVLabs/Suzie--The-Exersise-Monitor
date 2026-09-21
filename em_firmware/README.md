This repository is shared for private review only. Please do not copy, redistribute, or reuse the code without permission.
# Suzie

Suzie is a wearable exercise-analytics project focused on measuring workout performance from motion data. The goal is to build a compact, body-worn device that can be used on the arm, wrist, or leg and send useful workout metrics to a phone or companion app.

## Current Direction

Suzie is now primarily a **motion-first wearable** rather than a smartwatch-style device.

The wearable itself does not need a display. Instead, it focuses on sensing, processing, and transmitting workout data over BLE while the phone/app handles visualization and history.

The current priority is reliable workout structure and performance tracking rather than trying to measure every possible fitness metric.

## Core Metrics

Current and planned core metrics include:

- Set detection
- Movement / rep-event detection
- Set duration
- Active exercise time
- Rest / downtime
- Active-to-rest ratio
- Movement pace / interval timing
- Movement consistency
- Performance relative to an earlier baseline in the session
- Performance irregularities and output drop-off over time

Exact rep counting can still be useful, but it is not the only or primary measure of workout quality.

## Set and Timing Logic

Suzie separates a workout set into active movement and downtime instead of treating the entire period as active exercise.

The firmware currently uses:

- Movement events / phases to identify activity
- Timestamps to measure timing
- An inactivity alarm to determine when a set has ended
- Separate active-time and downtime measurements
- An active-to-rest ratio as one measure of workout efficiency

This makes it possible to preserve rest information without allowing downtime to distort the measured duration of actual exercise.

## Performance Gauge

Rather than using a generic fatigue score, Suzie is moving toward a **performance gauge** based on how current movement compares with the user's own earlier performance.

Possible inputs include:

- Movement pace
- Movement output / intensity
- Range or consistency of motion
- Variation between movements
- Longer pauses or increasing downtime

The goal is to identify meaningful performance decline or irregularity without pretending to directly measure physiological fatigue.

## Hardware

Current hardware direction:

- ESP32 / ESP32-S3 class microcontroller
- BMI160 IMU
- LiPo battery
- BLE communication
- Minimal status indication such as an LED

The MAX30102 PPG sensor has been used in earlier prototypes, but heart-rate sensing is no longer a core requirement because Suzie is intended to work in multiple positions on the body, where reliable PPG placement cannot always be guaranteed.

The onboard display has been removed from the current design direction.

## Software

The firmware is written primarily in C and handles:

- IMU sensor acquisition
- Filtering and normalization
- Movement / phase detection
- Peak and threshold-based event detection
- Set-state logic
- Timestamp collection
- Active-time and downtime calculation
- Inactivity alarms
- Workout metric generation

Development tools may also be used to stream and visualize sensor data on a PC while tuning algorithms.

## Design Goals

Suzie should:

- Work in multiple positions on the body
- Avoid depending on a specific exercise when possible
- Keep the wearable small and power-efficient
- Prefer metrics that can be measured reliably from available sensors
- Avoid overlapping or competing metrics that describe the same behavior
- Move visualization and workout history to a phone/app rather than the wearable itself

## Current Status

Suzie is in active development.

Recent work has focused on:

- Improving movement and phase detection
- Measuring active time separately from downtime
- Determining when a set begins and ends
- Using inactivity timing to close a set
- Building a useful active-to-rest ratio
- Refining which performance metrics are worth keeping

## Planned Work

- Refine active-time and downtime tracking
- Improve set detection across different exercises
- Define the final performance-gauge inputs
- Improve BLE data transfer to the companion app
- Add workout logging and history
- Continue reducing unnecessary hardware and metrics
- Test the algorithms across different body positions and movement types
