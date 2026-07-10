# Phase 6 verification: events and local history

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

Result: **22/22 tests passed, 0 failures.** New tests:

- `event_manager`: activation, active-alarm deduplication, severity escalation, acknowledgement, clearing, and no duplicate clear.
- `local_storage`: append/recover, sequence preservation, CRC corruption rejection, and bounded in-memory history.

## Implemented behavior

- Active events are keyed by light short address and event family.
- Repeated active alarms are suppressed.
- A higher severity emits an escalation transition.
- Clear transitions preserve the event snapshot and acknowledgement state.
- Reminder transitions are available after a configurable interval.
- Event history uses a fixed 128-record ring and CRC16; invalid records are skipped during recovery.
- The history API is intentionally storage-backend-neutral, allowing an ESP-IDF NVS/flash adapter without coupling event logic to flash writes.

## Firmware integration status

The `event_manager` and `local_storage` components compile in the ESP-IDF dependency graph. The NVS metadata adapter, batched flash checkpoint task, and live event-to-C6 handoff remain later integration work.

## Not yet validated

- Flash endurance and power-loss/torn-write recovery on the target partition.
- NVS encryption and production key management.
- Full event volume under a 64-device fault storm.
