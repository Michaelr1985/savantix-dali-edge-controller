# Phase 5 verification: local health and trend intelligence

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

Result: **20/20 tests passed, 0 failures.** New tests:

- `light_health`: warning/fault persistence, healthy recovery, direct lamp-failure precedence, health score deductions, and non-instant recovery.
- `trend_engine`: Welford mean/standard deviation, min/max, seven-day and thirty-day time windows, EWMA, and rate-of-change detection.

## Implemented behavior

- Persistence defaults support warning at 3 abnormal samples, fault at 5 consecutive abnormal samples, and recovery after 10 healthy samples.
- Direct lamp and gear flags produce failed state and score deductions before statistical inputs.
- Health score recovery is gradual and does not jump to 100 after one healthy reading.
- Rolling baselines use bounded storage and Welford statistics without dynamic allocation.
- Trends use monotonic timestamps and can be exercised with accelerated virtual time.

## Firmware integration status

The `light_health_engine` and `trend_engine` components are in the ESP-IDF dependency graph and compile as C++17 components. Per-light baseline persistence, event emission, and live task wiring remain later integration work.

## Not yet validated

- Field-calibrated electrical baselines by commanded-level band.
- Seven-day and thirty-day behavior using real operating data.
- Hardware temperature and power sensor calibration.
