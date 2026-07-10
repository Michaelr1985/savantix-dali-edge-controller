# Savantix Smart-Light DALI Edge Controller

Production-oriented ESP-IDF firmware for an ESP32-S3 DALI/DALI-2 edge controller. The completed project will discover and monitor DALI control gear, learn per-light baselines, detect faults and deterioration locally, and send only meaningful events or requested summaries to a future ESP32-C6 peer.

## Current implementation status

Phases 1 through 5 are implemented:

- Native ESP-IDF v6.0.2 project targeting ESP32-S3.
- Central Kconfig-backed controller configuration.
- Strong DALI address types and validity-bearing measurements.
- Hardware-independent DALI PHY interface.
- Allocation-free deterministic simulated PHY with bounded response queue.
- Monotonic time abstraction and optional Unix-time synchronisation.
- Validated boot-composition boundary.
- Standard short, group, and broadcast DALI forward-frame construction.
- Typed backward-response and basic-status decoding.
- Stable 64-entry priority command queue.
- Serial transactions with bounded retries, jittered stepped backoff, and preserved PHY failure causes.
- Saturating communication-quality counters with explicit no-data state.
- One-request-at-a-time protocol pipeline.
- Fixed 64-slot pre-addressed device registry with validated short addresses.
- Presence discovery with duplicate-address suspicion, three-scan missing hysteresis, recovery events, and bus-failure suppression.
- Core capability probing with explicit supported/unsupported states and suppression of repeated unavailable queries.
- Bounded adaptive scheduler with on/off/warning periods, observation windows, round-robin spreading, and normal/diagnostic bus-utilisation admission.
- Rule-based health scoring, persistence/recovery hysteresis, rolling baselines, and accelerated time-window trend analysis.
- Host CTest suite and verified ESP-IDF build.

Events, persistent history, the C6 framing session, and the four-light integrated demo are delivered in later phases. The current boot log does not claim those behaviors.

## Electrical safety boundary

**Never connect ESP32-S3 GPIO pins directly to a DALI bus.** DALI requires a compliant, isolated physical interface between the controller and the two-wire bus. GPIO assignments default to disabled (`-1`) until the transceiver and board design are selected and validated.

This firmware is not safety-rated. Underground deployment requires electrical design review, isolation and EMC verification, environmental qualification, DALI conformance tests, risk assessment, and site acceptance testing.

## Required toolchain

- ESP-IDF v6.0.2
- ESP32-S3 toolchain installed by ESP-IDF
- CMake 3.20 or later for host tests
- A C++17 host compiler

Example SDK setup on macOS or Linux:

```bash
git clone --branch v6.0.2 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf-v6.0.2
cd ~/esp/esp-idf-v6.0.2
./install.sh esp32s3
. ./export.sh
```

## Build the firmware

From the repository root:

```bash
. "$HOME/esp/esp-idf-v6.0.2/export.sh"
idf.py set-target esp32s3
idf.py build
```

Flash and monitor only after a supported ESP32-S3 board is connected:

```bash
idf.py -p PORT flash monitor
```

Replace `PORT` with the board's serial port. Press `Ctrl-]` to exit the monitor.

## Run host tests

```bash
cmake -S test/host -B build/host -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
```

The host suite compiles production domain and simulator code directly. It does not use a disconnected copy of runtime logic.

## Phase 1 boot behavior

With `CONFIG_SAVANTIX_DEMO_ENABLE=y`, boot validates configuration, initialises `SimulatedDaliPhy`, verifies its health, then performs one scripted `QUERY STATUS` protocol self-check against simulated address 3. This proves framing, queueing, transport, response parsing, and quality accounting; it is not device discovery or continuous polling.

With demo mode disabled, Phase 1 reports that no hardware PHY is selected and stops initialisation safely. A real isolated PHY adapter is introduced only after its electrical and timing assumptions are known.

## Configuration

Run `idf.py menuconfig` and open **Savantix DALI Edge Controller**. Phase 1 exposes:

- DALI TX/RX GPIO placeholders.
- Bus-power and transceiver-fault inputs.
- Optional bus-voltage and bus-current ADC inputs.
- ESP32-C6 UART GPIO placeholders.
- Maximum light count.
- Initial status polling, retry, bus-utilisation, thermal, and persistence defaults.
- C6, demo, and future anomaly-interface feature switches.

Optional GPIOs remain invalid until explicitly configured. Analog readings remain invalid until hardware calibration exists.

## Repository layout

```text
main/                    Boot composition and Kconfig adapter
components/dali_core/    Portable domain types
components/dali_phy/     Hardware-independent PHY contract
components/dali_simulator/ Deterministic simulated PHY
components/dali_protocol/ Frames, parser, queue, transactions, query client and quality
components/dali_device_manager/ Bounded device registry, discovery and capabilities
components/dali_scheduler/   Adaptive polling periods and bus-load admission
components/light_health_engine/ Rule-based scoring and recovery
components/trend_engine/     Rolling statistics and time-window trends
components/time_service/ Monotonic and optional wall-clock time
test/host/               Portable CTest suite
docs/superpowers/        Approved architecture and implementation plans
docs/verification/       Recorded verification evidence
```

## Design documents

- `docs/superpowers/specs/2026-07-10-savantix-dali-edge-controller-design.md`
- `docs/superpowers/plans/2026-07-10-savantix-dali-edge-controller-master-plan.md`
- `docs/superpowers/plans/2026-07-10-savantix-dali-phase-1-foundation.md`
- `docs/superpowers/plans/2026-07-10-savantix-dali-phase-2-protocol.md`
- `docs/superpowers/plans/2026-07-10-savantix-dali-phase-3-devices.md`
- `docs/verification/phase-1.md`
- `docs/verification/phase-2.md`
- `docs/verification/phase-3.md`
- `docs/verification/phase-4.md`
- `docs/verification/phase-5.md`

## Known Phase 1 limitations

- No real DALI physical-layer implementation exists yet.
- No continuous polling, diagnostics, storage, or C6 frames are emitted yet.
- The scheduler is currently a portable component; Phase 8 will connect it to FreeRTOS task ownership and the DALI pipeline.
- Hardware flashing and execution were not performed because no target board or isolated DALI transceiver was supplied.
- GPIO polarity, DALI timing, ADC scaling, stack sizes, and physical fault inputs require board-level validation.
- ESP-IDF v6.0.2 emits non-fatal configuration notifications from a few upstream component defaults; the Savantix project build itself completes successfully.
