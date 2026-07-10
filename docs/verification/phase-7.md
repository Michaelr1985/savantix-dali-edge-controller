# Phase 7 verification: C6 transport boundary

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

Result: **23/23 tests passed, 0 failures.** New `c6_interface` coverage proves frame encode/decode, start/end markers, sequence/type/payload preservation, CRC rejection, mock transport delivery, acknowledgement state, and status-request construction.

## Frame contract

```text
0x7E | version | message type | sequence LE | payload length LE |
payload | CRC16 LE | 0x7F
```

Payloads are bounded at 240 bytes. The codec has no heap allocation and is independent of UART, SPI, or Thread. The session exposes a transport interface so a future UART with RTS/CTS can replace the mock without changing event or protocol logic.

## Forwarding policy

The C6 boundary accepts event/status/summary/discovery/bus/heartbeat message types. No API accepts raw polling samples as a streaming operation. Normal unchanged readings therefore remain local by construction.

## Firmware integration status

The `c6_interface` component compiles in the ESP-IDF dependency graph. UART driver wiring, inbound request decoding, offline queue persistence, and full event-to-C6 task ownership remain final integration work.

## Not yet validated

- UART electrical pins, framing under noise, RTS/CTS timing, and peer firmware.
- Retry timing under a disconnected C6.
- End-to-end event transmission on hardware.
- Thread networking, which remains intentionally excluded.
