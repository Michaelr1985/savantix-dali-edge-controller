# Phase 2 DALI Protocol Verification

**Date:** 2026-07-10  
**Target:** ESP32-S3  
**ESP-IDF:** v6.0.2  
**Branch:** `codex/dali-edge-controller`

## Implemented behavior

- Standard 16-bit forward frames for short, group, and broadcast addressing.
- Direct-arc mask value 255 rejected by the `ArcLevel` type.
- Typed backward responses and basic DALI status-bit parsing.
- Fixed 64-entry stable priority queue.
- Serial transmit/receive transactions with response timeout handling.
- Retryable timeout, collision, and busy outcomes.
- Non-retryable bus-power, transceiver, initialisation, argument, and internal errors.
- Stepped retry delays of 100 ms, 500 ms, then 2000 ms plus 0-25 ms injected jitter.
- Maximum eight configured retries enforced defensively; project default remains three.
- Saturating communication-quality counters and explicit no-data state.
- One-request protocol pipeline and one simulated boot self-check.

## Red-green evidence

| Behavior | Observed red state | Observed green state |
| --- | --- | --- |
| Frame construction | Missing `dali_commands.h` | Six standard address/frame encodings and arc-mask rejection passed |
| Response parsing | Missing parser header | Yes/no, status bits, timeout, collision and hardware-fault quality passed |
| Command queue | Missing queue header | Priority, FIFO stability, empty and capacity behavior passed |
| Transactions | Missing transaction header | Success, no-response, retries, backoff, exhaustion and terminal faults passed |
| Communication quality | Missing tracker type | Counters, failures, success rate and no-data state passed |
| Protocol pipeline | Missing pipeline header | Queue-to-transaction-to-parser integration passed |

## Host verification

Commands:

```bash
cmake --fresh -S test/host -B build/host -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
```

Expected test inventory after Phase 2:

1. `domain_types`
2. `phy_contract`
3. `test_simulated_dali_phy`
4. `test_simulated_dali_phy_capacity`
5. `time_contract`
6. `foundation_composition`
7. `dali_frame`
8. `dali_response_parser`
9. `dali_command_queue`
10. `dali_transaction`
11. `communication_quality`
12. `dali_protocol_pipeline`

Recorded clean result: 12 of 12 passed, 0 failed. Total CTest time was 8.31 seconds.

## Firmware verification

Commands:

```bash
. "$HOME/esp/esp-idf-v6.0.2/export.sh"
idf.py -B build/idf fullclean
idf.py -B build/idf build
```

Expected artifact: `build/idf/savantix_dali_edge_controller.bin`.

Recorded clean result: build exit 0. The integrated Phase 2 image measured `0x29400` bytes with `0x1d6c00` bytes (92%) free in the 2 MB app partition. The bootloader measured `0x5240` bytes with 36% of its partition free.

## Retry and failure semantics

`maxRetries` counts additional attempts. The default value of three permits at most four total attempts. Delay is applied only between retryable attempts, never after the last failure. `BusPowerLost`, `TransceiverFault`, `NotInitialised`, `InvalidArgument`, and `InternalError` stop immediately. The final `PhyResult` remains available to device and bus-failure logic in later phases.

A valid returned byte of `0x00` remains a valid zero value. It is not treated as timeout or unsupported data.

## Hardware validation still required

- Forward/backward bit timing and settling delays with the selected isolated transceiver.
- Collision and bus-busy sensing behavior on the real interface.
- DALI electrical conformance and response-window measurement.
- Randomised retry behavior under multi-master bus contention.
- Bench validation against representative DALI and DALI-2 gear.

The simulator proves software state transitions, not physical DALI conformance.
