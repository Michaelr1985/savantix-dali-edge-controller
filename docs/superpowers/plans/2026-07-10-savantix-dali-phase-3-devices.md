# Savantix DALI Phase 3 Device Management Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Discover already-addressed DALI gear, maintain bounded per-light records, detect new/missing/recovered/duplicate-address conditions, and suppress unsupported capability probes.

**Architecture:** `dali_device_manager` is portable C++17 and depends on an `IDaliQueryClient` rather than FreeRTOS or the PHY. A protocol adapter executes synchronous queries through the Phase 2 pipeline. The manager owns a fixed 64-slot registry indexed by short address and returns bounded change batches. Three failed presence scans are required before a known device becomes missing.

**Tech Stack:** C++17, CMake/CTest, ESP-IDF v6.0.2, Phase 2 protocol pipeline

---

### Task 1: Add a typed synchronous query client

**Files:**
- Create: `components/dali_protocol/include/dali/protocol/i_dali_query_client.h`
- Create: `components/dali_protocol/include/dali/protocol/pipeline_query_client.h`
- Create: `components/dali_protocol/pipeline_query_client.cpp`
- Create: `test/host/test_pipeline_query_client.cpp`

- [ ] Write a failing test that scripts a valid response, timeout exhaustion, collision exhaustion, and bus-power loss. Verify `DaliQueryOutcome` preserves `Ok`, `NoResponse`, `Collision`, and `BusFailure` separately.
- [ ] Run the target and observe the missing query-client header.
- [ ] Define `DaliQueryResult { outcome, Reading<uint8_t> response, attempts, PhyResult }` and `IDaliQueryClient::query(ShortAddress,DaliCommand,priority)`. Implement `PipelineQueryClient` by enqueueing one request, processing it immediately, and mapping the final transaction without treating valid zero as absent.
- [ ] Run all tests and commit `feat: add typed DALI query client`.

### Task 2: Create the fixed device registry

**Files:**
- Create: `components/dali_device_manager/CMakeLists.txt`
- Create: `components/dali_device_manager/include/dali/device/dali_device_manager.h`
- Create: `components/dali_device_manager/dali_device_manager.cpp`
- Create: `test/host/test_dali_device_registry.cpp`

- [ ] Write a failing test for empty lookup, insertion at addresses 0 and 63, idempotent lookup, count, and rejection of an invalid raw address before it reaches the registry.
- [ ] Observe the missing manager header.
- [ ] Define `DeviceLifecycle { Unknown, Present, Missing, DuplicateAddress }`, `CapabilityState { Unknown, Supported, Unsupported }`, `DaliCapabilityMap`, and `DaliDeviceRecord` containing address, lifecycle, present flag, missed-presence count, first/last seen uptime, capabilities, realtime/lifetime/health data, device type, version, and physical minimum readings. Use `std::array<std::optional<DaliDeviceRecord>,64>`.
- [ ] Verify and commit `feat: add bounded DALI device registry`.

### Task 3: Discover and track lifecycle changes

**Files:**
- Modify: `components/dali_device_manager/include/dali/device/dali_device_manager.h`
- Modify: `components/dali_device_manager/dali_device_manager.cpp`
- Create: `test/host/test_dali_discovery.cpp`

- [ ] Write a failing fake-client test: address 0 and 3 respond, address 7 collides, others time out. Expect two `DeviceDiscovered` changes and one `DuplicateAddressSuspected`, with two present records. On a second identical scan, expect no duplicate discovery events. Then make address 3 time out for three scans and expect one `DeviceMissing`; restore it and expect one `DeviceRecovered`.
- [ ] Observe missing `scanPresence` behavior.
- [ ] Implement `DiscoveryChangeType`, fixed `DiscoveryBatch` of at most 64 changes, and `scanPresence(nowMs)`. Query `QueryControlGearPresent` for addresses 0-63 at high priority. Valid response marks present regardless of byte value. Collision creates a duplicate suspicion without creating a normal device. Known present devices require three consecutive no-response scans before missing. Bus failures do not increment per-device missing counters.
- [ ] Verify and commit `feat: discover pre-addressed DALI gear`.

### Task 4: Detect and suppress unsupported capabilities

**Files:**
- Modify: `components/dali_device_manager/include/dali/device/dali_device_manager.h`
- Modify: `components/dali_device_manager/dali_device_manager.cpp`
- Create: `test/host/test_dali_capabilities.cpp`

- [ ] Write a failing test for a new device where basic status, device type, version, and physical minimum respond; verify supported states and values. Make a probe time out twice and verify its state becomes unsupported. Run capability detection again and verify the fake client's count for that unsupported command does not increase.
- [ ] Observe missing capability API.
- [ ] Implement `CapabilityFeature` entries for basic status, device type, version, physical minimum, energy, power, current, temperature, operating hours, failure counters, diagnostics, and identification. Each feature tracks state and consecutive no-response count. Explicit valid response sets supported; two isolated no-responses set unsupported; bus failures leave state unchanged. `detectCoreCapabilities(address)` probes only unknown states.
- [ ] Verify and commit `feat: detect DALI gear capabilities`.

### Task 5: Integrate and document Phase 3

**Files:**
- Modify: `main/CMakeLists.txt`
- Modify: `main/app_main.cpp`
- Modify: `README.md`
- Create: `docs/verification/phase-3.md`
- Create: `test/host/test_device_manager_integration.cpp`

- [ ] Write a failing integration test using a scripted fake query client with four present addresses, one collision, and remaining timeouts. Verify discovery batch, registry count, core capability values, and repeated-scan deduplication.
- [ ] Keep the boot demo at the verified one-query protocol self-check; exercise the 64-address scan through the deterministic host integration test until Phase 8 adds an address-aware multi-device simulator.
- [ ] Run a fresh host build, all CTests, clean ESP-IDF build, and `git diff --check`.
- [ ] Record test inventory, image size, lifecycle rules, and remaining commissioning limitations; mark this plan complete and commit `feat: complete DALI device management phase`.

## Completion gate

- [ ] Registry capacity is fixed at 64 and indexed by validated short address.
- [ ] Valid zero responses prove presence.
- [ ] Three isolated failures are required for missing state.
- [ ] Bus-wide failures do not create one missing alarm per light.
- [ ] Collision remains distinguishable as duplicate-address suspicion.
- [ ] Unsupported capabilities stop being queried.
- [ ] Fresh host tests and ESP-IDF build pass with committed evidence.
