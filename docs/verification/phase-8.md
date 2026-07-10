# Phase 8 verification: integration and demo

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

Result: **27/27 tests passed, 0 failures.** New tests:

- `system_monitor`: task heartbeat timeout and queue high-water tracking.
- `demo_controller`: discovery of four simulated lights, event-to-C6 output, lamp failure, rising thermal warning, and health-score changes.
- `diagnostics`: direct DALI fault precedence, thermal thresholds, and electrical drift classification.
- `baseline_engine`: five commanded-level bands, healthy-only updates, and explicit validity handling.

## Firmware verification

```text
. "$HOME/esp/esp-idf-v6.0.2/export.sh"
idf.py build
```

The final build must include `demo_controller`, `system_monitor`, `event_manager`, `local_storage`, `c6_interface`, scheduler, health, trend, device, protocol, simulator, and time components. The application remains below the 2 MB app partition; flash size and exact binary size are recorded from the final build output.

## Runtime task boundary

The demo task uses a FreeRTOS task with a one-second cadence and monotonic ESP timer. It does not write physical outputs, use Thread, or send raw samples. A future hardware deployment replaces the simulator and mock C6 transport behind their existing interfaces.

## Final known limitations

- No selected isolated DALI transceiver adapter is present; GPIO/RMT timing remains a hardware-specific implementation boundary.
- The local history ring is CRC-protected RAM logic; NVS metadata is available, but full batched flash checkpoint orchestration remains a deployment hardening task.
- The C6 transport is mock-only; UART electrical integration and Thread forwarding are excluded.
- No physical ESP32-S3 board was supplied, so flashing, serial logs, EMC, isolation, and field FAT/SAT evidence remain open.
