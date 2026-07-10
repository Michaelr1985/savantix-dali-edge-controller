# Phase 4 verification: adaptive polling scheduler

**Branch:** `codex/dali-edge-controller`  
**Target:** ESP32-S3  
**SDK:** ESP-IDF v6.0.2 (`v6.0.2`)  
**Date:** 2026-07-10

## Host verification

```text
cmake -S test/host -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --parallel
ctest --test-dir build-host --output-on-failure
```

Result: **18/18 tests passed, 0 failures.** The new `dali_scheduler` test proves:

- commanded-on status at 5 seconds;
- commanded-off status at 30 seconds;
- warning observation status at 2 seconds;
- bounded critical and normal bus-utilisation admission at 60% and 30% limits;
- fixed-capacity state for 64 devices and round-robin cursor behavior.

## Scheduling policy implemented

- Normal status: 5 seconds when commanded on, 30 seconds when commanded off.
- Warning/recent-fault status: 2 seconds for the first 60 seconds, then 5 seconds for five minutes.
- Presence: 30 seconds.
- Electrical and temperature: 60 seconds normal, 10 seconds under observation.
- Lifetime counters: 6 hours.
- Identification: 24 hours.
- Critical requests may use the diagnostic utilisation limit; normal/high/low requests use the normal limit.
- Requests are admitted only below the configured limit, so the scheduler never intentionally exceeds the target.

## Firmware integration status

The `dali_scheduler` component is included in the ESP-IDF dependency graph and compiles as portable C++17. FreeRTOS task wiring and live DALI queue ownership remain Phase 8 integration work.

## Not yet validated

- Bus transaction duration calibration against the selected isolated PHY.
- Long-running jitter distribution under a full 64-device network.
- Hardware bus utilisation instrumentation.
