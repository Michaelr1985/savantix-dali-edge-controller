# Savantix DALI Edge Controller Master Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver the complete ESP32-S3 DALI edge controller described by the approved specification as eight compiling, tested, backward-compatible increments.

**Architecture:** A portable C++17 domain core contains protocol, scheduling, diagnostics, statistics, health, events, and framing. ESP-IDF adapters own FreeRTOS, NVS, flash partitions, watchdog, logging, RMT/GPIO, and UART. Each phase adds one vertical capability, preserves all earlier tests, and ends with host and ESP-IDF build evidence.

**Tech Stack:** ESP-IDF v6.0.2, ESP32-S3, C++17, CMake, CTest, ESP-IDF Unity, FreeRTOS, NVS, custom flash partition, RMT, UART

---

## Phase sequence

| Phase | Independent result | Detailed plan |
| --- | --- | --- |
| 1. Foundation | Compilable ESP-IDF project, central configuration, domain types, time abstraction, bounded simulator PHY, host tests | `2026-07-10-savantix-dali-phase-1-foundation.md` |
| 2. Protocol | Address/frame codec, backward response handling, serial transaction service, timeouts, retries, priority queue, quality counters | Written after Phase 1 interface verification |
| 3. Devices | Pre-addressed discovery, address verification, lifecycle, capabilities, identity and per-light records | Written after Phase 2 interface verification |
| 4. Scheduling | Adaptive due times, round-robin spreading, jitter, diagnostic windows, utilisation admission and retry backoff | Written after Phase 3 interface verification |
| 5. Intelligence | Baselines by level band, rolling statistics, persistence, hysteresis, fault rules, health recovery, deterministic trends | Written after Phase 4 interface verification |
| 6. Events and storage | Alarm lifecycle, snapshots, deduplication, escalation, acknowledgement, reminders, NVS metadata, circular history | Written after Phase 5 interface verification |
| 7. C6 boundary | Binary frames, CRC16, sequence/ack/retry session, request/response, bounded offline buffering, mock transport | Written after Phase 6 interface verification |
| 8. Integration | FreeRTOS task wiring, system monitoring, four-light demo, complete simulator scenarios, stability tests, documentation and audit | Written after Phase 7 interface verification |

## Cross-phase invariants

- [ ] Production behavior is introduced only after a test has failed for the expected missing behavior.
- [ ] `dali_core` remains independent of ESP-IDF and FreeRTOS headers.
- [ ] Only the bus service calls `IDaliPhy` after Phase 2.
- [ ] All queues, stores, histories, and device collections are bounded.
- [ ] Unsupported or unavailable readings carry explicit validity and never become numeric zero implicitly.
- [ ] Direct DALI faults outrank statistical inference.
- [ ] Normal unchanged samples never enter the C6 outbound queue.
- [ ] New phases preserve all earlier passing host and ESP-IDF tests.
- [ ] Every phase records exact build/test output in `docs/verification/phase-N.md`.
- [ ] Hardware-only claims remain labelled unverified until isolated DALI bench evidence exists.

## Phase completion command set

Every phase must run these commands from the repository root:

```bash
cmake -S test/host -B build/host -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
idf.py set-target esp32s3
idf.py build
git diff --check
```

The ESP-IDF Unity test application is added in the first phase that needs platform behavior and is then included in every later command set. No phase is described as compiling under ESP-IDF unless `idf.py build` exits successfully under v6.0.2.

## Final acceptance audit

Phase 8 produces `docs/verification/requirements-audit.md`, with one row for every numbered brief section and named deliverable. Each row records the implementing file, automated test or hardware test, documentation source, result, and any external validation dependency. The goal is complete only when all software-scope rows have direct passing evidence and all physical-scope rows are explicitly separated as deployment prerequisites.
