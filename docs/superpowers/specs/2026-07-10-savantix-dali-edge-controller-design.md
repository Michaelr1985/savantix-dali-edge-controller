# Savantix Smart-Light DALI Edge Controller Design

**Date:** 2026-07-10  
**Status:** Approved architecture  
**Target:** ESP32-S3, ESP-IDF v6.0.2, C++17  
**Generator mode:** Full Project Pack

## 1. Executive summary

The Savantix Smart-Light DALI Edge Controller is an ESP32-S3 firmware application that owns DALI and DALI-2 communication, discovers and monitors control gear, learns per-light operating baselines, detects faults and deterioration, and emits only useful events or requested summaries to a future ESP32-C6 transport peer. Raw unchanged polling data remains local. Thread networking is explicitly outside this release.

The design uses a portable C++17 domain core surrounded by narrow ESP-IDF adapters. Pure protocol, scheduling, statistics, health, trend, event, and framing logic can therefore run in host tests. FreeRTOS, NVS, flash, watchdog, GPIO/RMT, UART, and ESP logging remain in platform components. A simulated DALI PHY provides deterministic development and demonstration before the isolated physical interface is finalised.

## 2. Confirmed facts

- The controller is an ESP32-S3 using ESP-IDF and CMake.
- C++17 is the preferred implementation language.
- The bus is DALI/DALI-2 and must be connected through an isolated, compliant physical interface.
- Lights may already have valid short addresses; full commissioning is a future extension.
- Feature availability differs by control gear and must be detected before advanced polling.
- The controller must work correctly with partial data and must never interpret unavailable data as zero.
- Direct DALI failure flags take priority over statistical predictions.
- Local intelligence includes adaptive polling, persistence and hysteresis, baseline learning, trend detection, health scoring, event deduplication, and gradual recovery.
- Normal unchanged readings remain local.
- The ESP32-C6 boundary uses framed messages with CRC, sequence numbers, acknowledgements, and retries. The initial transport is a log-producing mock.
- Thread networking and embedded machine learning are outside the first release.
- The demonstration simulates four lights: healthy, lamp failure, rising temperature, and intermittent communication.

## 3. Assumptions

- ESP-IDF v6.0.2 is the build baseline. SDK-dependent code is isolated to permit a later 5.x backport if required.
- The first real DALI PHY will use the ESP32-S3 RMT peripheral unless hardware validation selects another peripheral. The protocol layer does not depend on that choice.
- The firmware supports at most 64 addressed control gears, with the actual limit configurable downward.
- The first release observes already commissioned gear. It verifies addresses and detects duplicates, missing devices, new devices, and suspected replacements, but does not assign short addresses.
- Advanced DALI-2 diagnostics are queried only through declared capability probes. Unsupported commands are suppressed after detection.
- Safety-critical underground deployment requires hardware engineering review, EMC testing, isolation verification, environmental qualification, and site acceptance testing. This firmware is not a safety-rated control system.
- Wall-clock time can arrive later from the C6. Until then, records use monotonic uptime and are marked as lacking authoritative Unix time.

## 4. Scope and release boundaries

### Included

- Modular ESP-IDF project, CMake manifests, Kconfig, and configuration defaults.
- DALI PHY abstraction, simulated PHY, and a replaceable real-hardware adapter boundary.
- Forward-frame construction, backward-frame parsing, timeouts, validation, collisions, retries, backoff, counters, quality tracking, prioritised queues, rate limits, and utilisation control.
- Broadcast, group, and short addressing helpers.
- Discovery of pre-addressed gear, presence verification, capability detection, duplicate/missing/new/replacement detection, and per-light records.
- Adaptive, spread polling with diagnostic escalation and bus-load control.
- Rule-based diagnostics, persistence windows, hysteresis, baseline learning, rolling statistics, deterministic trends, feature extraction, health scoring, and gradual recovery.
- Event lifecycle, snapshots, deduplication, acknowledgement state, clearing, escalation, reminders, and occurrence counters.
- NVS-backed metadata and a wear-conscious circular history store with RAM aggregation.
- Time abstraction, task heartbeats, queue-overflow detection, reset-reason logging, and controlled subsystem recovery.
- C6 binary framing, CRC16, sequence tracking, retry/acknowledgement state, request/response messages, and mock transport.
- Host tests, ESP-IDF component tests, simulated fault scenarios, demonstration mode, architecture and operator/developer documentation.

### Excluded

- Thread networking on the ESP32-C6.
- DALI short-address assignment or destructive commissioning.
- A production physical-layer schematic or direct electrical connection to the DALI bus.
- TensorFlow Lite or another trained anomaly model.
- Cloud services, remote firmware update, production security provisioning, or a graphical HMI.

## 5. Architectural approach

The approved approach is a staged vertical slice with a portable core and ESP-IDF adapters.

```text
Isolated DALI PHY / simulator
            |
            v
      DALI bus service <---- bounded priority command queues
            |
            v
      device manager <----> adaptive scheduler
            |
            v
       per-light state
        /     |      \
       v      v       v
 diagnostics trends  storage
       \      /
        v    v
      event manager ----> C6 frame codec ----> mock/UART transport

system monitor observes task heartbeats, queues, storage, bus, and C6 link
```

Only the DALI bus service may call `IDaliPhy`. Other modules request work through bounded queues and consume typed results. State mutation has a named owner; snapshots cross task boundaries by value or through explicit protected stores. No network or storage callback may drive the DALI PHY directly.

## 6. Project structure

```text
SLS/
├── CMakeLists.txt
├── Kconfig.projbuild
├── sdkconfig.defaults
├── partitions.csv
├── README.md
├── main/
│   ├── CMakeLists.txt
│   ├── app_main.cpp
│   ├── app_config.cpp
│   └── app_config.h
├── components/
│   ├── dali_core/
│   ├── dali_phy/
│   ├── dali_protocol/
│   ├── dali_device_manager/
│   ├── dali_scheduler/
│   ├── dali_diagnostics/
│   ├── light_health_engine/
│   ├── trend_engine/
│   ├── event_manager/
│   ├── local_storage/
│   ├── c6_interface/
│   ├── time_service/
│   ├── system_monitor/
│   └── dali_simulator/
├── test/
│   ├── host/
│   └── idf/
├── docs/
└── tools/
```

Each component owns one responsibility and exposes a documented public `include/` interface. Common domain types live in `dali_core`; it contains no ESP-IDF, FreeRTOS, storage, logging, or hardware dependencies.

## 7. Core interfaces and ownership

### DALI physical layer

`IDaliPhy` supplies `init`, `transmit`, `receive`, `isBusBusy`, `hasCollision`, and controlled reset/health reporting. Implementations include a deterministic simulator and an RMT-oriented placeholder/adapter. GPIO, timing, polarity, bus-power input, transceiver-fault input, optional voltage ADC, and optional current ADC originate in central configuration.

### Protocol and bus service

The protocol component constructs address and command frames and validates backward responses. The bus service serialises transactions, enforces bus-idle timing and utilisation budgets, tracks occupancy and quality, applies retry/backoff with jitter, and reports typed results. Fixed-capacity priority queues prevent unbounded memory use.

### Device manager

The device manager scans short addresses 0-63, verifies responses, detects duplicate or inconsistent identity, creates fixed-capacity device records, and manages lifecycle transitions. Capability probes populate a per-device map and disable unsupported polls. Identity changes at an existing address produce replacement suspicion rather than silently overwriting history.

### Scheduler

The scheduler stores independent due times for status, presence, electrical, temperature, counters, identity, and diagnostic follow-up. It spreads load by round robin and deterministic jitter. It admits work only when the current rolling utilisation budget permits it, except bounded critical verification work.

### Data model

Every optional measurement is represented as a validity-bearing value containing value, quality, and timestamps. Realtime state, lifetime counters, capability map, learned baselines, health state, communication quality, and fault history are separate structures. A missing or unsupported value cannot participate in calculations.

### Diagnostics and health

The diagnostics engine evaluates direct flags first, then persistent rule windows. The health engine applies configured deductions, maps scores to states, and uses recovery hold time plus rate-limited score restoration. Fault clearing never removes fault history.

### Trend engine

Five commanded-level bands hold independent rolling statistics for current, power, temperature, and response time. Baselines update only during stable, fault-free, thermally normal, communicatively healthy operation. A baseline becomes provisional after 100 valid samples and mature only after sufficient samples spanning seven operating days. Statistical warnings include confidence and require comparable operating conditions.

### Event manager

The event manager is the sole owner of active alarm lifecycle state. Its key is device address plus event family. Activation, escalation, reminder, acknowledgement, and clearing are distinct transitions. Every emitted event captures the measurements relevant at the transition.

### Storage

NVS stores low-churn configuration, capability, health, counters, baseline metadata, and last known fault state using versioned records and CRC. Recent readings, trends, events, and diagnostic snapshots use a bounded circular partition with record headers, sequence numbers, CRC, and recovery scanning. Writes are batched or change-driven.

### C6 interface

The codec is independent of UART. Frames contain start marker, protocol version, message type, sequence number, payload length, payload, CRC16, and end marker. The session layer handles acknowledgement, retry, duplicate sequence detection, bounded outbound buffering, requests, and offline operation. Normal samples never enter this queue.

### System monitor and time

The time service provides monotonic milliseconds and optional Unix time with validity. The monitor checks task heartbeats, queue high-water marks, bus lockup, stuck line indications, physical power/fault inputs, C6 session progress, storage health, and reset reason. Recovery targets the affected subsystem; a single missing light never reboots the controller.

## 8. Runtime task model

The initial task set is intentionally smaller than one task per component:

| Task | Responsibility | Relative priority |
| --- | --- | --- |
| `dali_bus_task` | Own PHY, execute transactions, enforce timing | Highest application priority |
| `control_task` | Device lifecycle, scheduling, diagnostics, health, events | High |
| `c6_interface_task` | Encode/decode frames, mock/UART transport, ack/retry | Medium |
| `storage_task` | Batch checkpoints and circular-log writes | Low |
| `maintenance_task` | Trends, summaries, watchdog and heartbeat audit | Low |

This avoids unnecessary task proliferation while preserving module boundaries. Queues are bounded and statically sized where practical. Task priorities and stack sizes are configured and measured. The ESP task watchdog is fed only after a successful work cycle.

## 9. Polling and bus-load policy

- Commanded on, normal: status every 5 seconds.
- Commanded off: status every 30 seconds.
- Warning/fault observation: every 2 seconds for 60 seconds, then every 5 seconds for 5 minutes, then normal if recovered.
- Healthy electrical values: every 60 seconds; observation: every 10 seconds.
- Temperature: every 60 seconds; near warning: every 10 seconds.
- Operating/start/power-cycle counters: every 6 hours.
- Failure counters: every hour.
- Identification: discovery, suspected replacement, and every 24 hours.
- Presence: every 30 seconds, scaled upward if necessary; three bounded immediate rechecks before missing declaration.
- Normal bus utilisation target: 30%; temporary diagnostic target: 60%.
- Retry delays: approximately 100 ms, 500 ms, and 2 seconds plus jitter, followed by degraded scheduled checking.

Admission uses measured/estimated transaction occupancy over a rolling time window. Per-device retry budgets prevent one failed fitting from monopolising the bus. Critical verification may pre-empt low-priority polls but remains bounded.

## 10. Diagnostic and alarm design

Initial rule families are confirmed light-source failure, possible low-output failure, device missing, complete bus failure, control-gear failure, intermittent fault, thermal warning/critical, and electrical drift. Direct confirmed DALI flags may activate immediately. Statistical warnings use configurable sample persistence, defaulting to 3 of 5 abnormal samples; statistical faults require 5 consecutive abnormal samples; recovery requires 10 consecutive healthy samples.

Temperature defaults use 80 °C activation and 75 °C clearing hysteresis. All thresholds and scoring weights come from configuration. A complete bus failure suppresses per-device alarm storms while preserving affected-device context. Alarm records include cause, severity, confidence, operator action, activation/clear state, acknowledgement, count, and snapshot.

## 11. Power and fault recovery

- Outputs and PHY controls enter a safe inactive state before normal startup.
- Versioned persistent records are validated before use; corrupt records fall back to defaults and produce a storage event.
- On reboot, retained alarms and device state are restored as historical context, then revalidated against live gear before being considered active.
- DALI bus lockup triggers a controlled PHY reset and bounded rediscovery, not an unconditional device reboot.
- C6 loss retains events locally in a bounded queue/history and does not stop DALI monitoring.
- Queue overflow increments a diagnostic counter and generates one deduplicated system event; it does not silently overwrite critical work.
- Unexpected reset reasons are persisted at the next safe checkpoint and exposed in summaries.

## 12. Configuration and IO definition

Kconfig provides DALI TX/RX GPIOs, peripheral selection, bus-power input, transceiver-fault input, optional voltage/current ADC inputs, C6 UART pins and flow control, polling intervals, retry limits, utilisation targets, diagnostic thresholds, persistence counts, maximum light count, log levels, C6 enable, demo mode, and anomaly-model interface enable.

All GPIO assignments default to disabled or clearly documented development values until real hardware is selected. Inputs use explicit polarity and fail-state metadata. Optional analog values are invalid until calibrated. The ESP32-S3 is never documented as directly connected to the DALI wires.

## 13. Test strategy

Development follows red-green-refactor. Pure domain logic is tested on the host first; integration with FreeRTOS, NVS, partition APIs, watchdog, and hardware adapters is tested through an ESP-IDF Unity test application.

Required automated coverage includes frame construction, addressing, response parsing, timeouts, retries, capability suppression, scheduling, load admission, baseline statistics, rolling windows, standard deviation, trends, score deductions and recovery, persistence and hysteresis, event lifecycle, serialisation, CRC rejection, storage recovery, and queue bounds.

The simulator covers healthy operation, lamp failure, gear failure, missing device, intermittent response, overheating, falling output current, rising input power, total bus failure, collision/corruption, and unsupported commands. The four-light demo proves discovery, polling, health, trend/persistence, deduplication, recovery, and mock C6 output. Long-running tests use accelerated virtual time to exercise seven- and thirty-day windows without wall-clock waiting.

Verification layers are:

1. Host CMake/CTest build with warnings treated as errors.
2. ESP-IDF `idf.py set-target esp32s3` and `idf.py build`.
3. ESP-IDF Unity test-app build and, when hardware is available, execution.
4. Demo trace assertions against expected event sequences.
5. Static checks for bounded queues, forbidden direct PHY ownership, unsupported-value handling, and accidental Thread dependencies.
6. Bench FAT on isolated DALI hardware before field use.

## 14. Delivery phases

Each phase ends with compiling software, new failing-then-passing tests, updated assumptions, and retained compatibility:

1. Foundation: project, configuration, core types, task shell, time abstraction, simulator PHY.
2. Protocol: frames, responses, transactions, retries, priority queue, quality counters.
3. Devices: discovery, lifecycle, addressing verification, capabilities, per-light records.
4. Scheduling: adaptive due times, spreading, utilisation, backoff, observation periods.
5. Intelligence: baselines, statistics, diagnostics, health score, trends, recovery.
6. Events and storage: event lifecycle, snapshots, persistent metadata, circular history.
7. C6 boundary: framing, CRC, session state, request/response, mock transport.
8. Integration: four-light demo, fault scenarios, stability tests, documentation, completion audit.

## 15. Documentation and handoff

The repository will include build/flash/test instructions; configuration reference; architecture; capability handling; bus-load calculation; health scoring; simulated scenarios; sample events; real-PHY replacement guidance; future C6 integration guidance; known limitations; FAT/SAT checklist; and next steps.

The code is native ESP-IDF source, not a vendor PLC import. Production suitability remains conditional on real transceiver validation, DALI conformance testing, memory/stack measurement, flash endurance testing, environmental qualification, security review, and site acceptance.

## 16. Acceptance criteria

The release is complete only when:

- The ESP-IDF application and test application compile for ESP32-S3 under v6.0.2.
- Host tests pass and cover every test category named in the project brief.
- The simulator reproduces every required fault scenario.
- Demo mode produces the required four-light lifecycle without duplicate active alarms and emits valid mock C6 frames.
- Every capability-gated unsupported query stops recurring.
- Bus utilisation and retry tests prove configured bounds.
- Storage recovery tests prove valid-record recovery after torn or corrupt records.
- No Thread implementation or continuous raw-reading forwarding exists.
- All 18 named deliverables in the brief are present and cross-referenced from the README.
- A requirement-by-requirement audit links each brief requirement to code, tests, documentation, or explicitly documented hardware-only validation.

## 17. Known design constraints

- DALI timing and electrical conformance cannot be proven without the selected isolated transceiver and bench instrumentation.
- Manufacturer-specific DALI-2 diagnostics require device documentation and test samples.
- Long-term health accuracy depends on representative field data and stable sensor calibration.
- ESP-IDF compilation requires installation of the selected SDK/toolchain; it is not currently available in the workspace shell.
- Hardware execution, flash endurance, EMC, and underground environmental qualification remain physical verification activities.
