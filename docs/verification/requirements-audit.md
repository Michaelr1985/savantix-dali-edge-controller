# Savantix DALI Edge Controller requirements audit

**Audited commit:** `f567ec4`  
**Branch:** `codex/dali-edge-controller`  
**Host evidence:** 27/27 CTest tests passed  
**Firmware evidence:** ESP-IDF v6.0.2 `idf.py build` passed for `esp32s3`

| Brief area | Evidence | Status |
| --- | --- | --- |
| 1-2. Objective and edge principle | `README.md`, demo controller, C6 forwarding policy | Software implemented; no raw streaming |
| 3. Modular architecture | `docs/superpowers/specs/2026-07-10-savantix-dali-edge-controller-design.md` and component tree | Verified |
| 4. PHY abstraction | `components/dali_phy`, simulator, isolated-PHY README boundary | Interface verified; real transceiver pending |
| 5. DALI communication | `components/dali_protocol`, frame/parser/queue/transaction tests | Verified for simulator |
| 6. Addressing/discovery | `components/dali_device_manager`, discovery/integration tests | Pre-addressed gear verified; commissioning deferred |
| 7. Capability map | device manager capability tests | Verified for core probes and suppression |
| 8. Per-light data model | `dali/core/types.h` validity-bearing records | Verified |
| 9-10. Polling and bus load | `components/dali_scheduler`, scheduler test | Policy verified; live task wiring limited to demo integration |
| 11-12. Failure rules and false-alarm persistence | `dali_diagnostics`, `light_health_engine`, health/diagnostic tests | Rule core verified |
| 13-14. Baselines and learning | `baseline_engine`, baseline test | Five bands and healthy-only updates verified; long-term field maturity pending |
| 15-17. Health/trends/ML boundary | health engine, trend engine, feature vector, trend tests | Deterministic core verified; ML intentionally not included |
| 18-19. Events and deduplication | `event_manager`, event test | Verified |
| 20. C6 interface | `c6_interface`, CRC/frame test, mock transport | Framing/session boundary verified; UART/Thread excluded |
| 21. Local storage | CRC ring, NVS metadata adapter, storage test | Software paths compile and recover records; flash endurance pending |
| 22. Time | `time_service`, time contract test | Verified |
| 23-24. Tasks/watchdog/recovery | demo FreeRTOS task, system monitor | Heartbeat/demo task verified; full production watchdog orchestration pending hardware integration |
| 25-26. Logging/config | ESP logging, Kconfig, README | Verified at build/configuration level |
| 27. Testing | 27 host tests, simulator and fault scenarios | Verified software suite |
| 28. Demo | `demo_controller`, demo test | Four-light scenario verified |
| 29. Deliverables | README, architecture, phase verification records, build artifacts | Software deliverables present |
| 30. Phased implementation | commits from foundation through integration | Verified |
| 31-33. Coding standards/assumptions | C++17, bounded arrays, validity flags, no Thread dependency, documented limitations | Verified with documented hardware exceptions |

## Final software evidence

```text
Host: 27/27 tests passed, 0 failures
ESP-IDF: v6.0.2, target esp32s3, Project build complete
App image: 0x2A280 bytes (172,672 bytes)
Smallest app partition: 0x200000 bytes
Reported free space: 0x1D5D80 bytes (92%)
```

## Explicit deployment prerequisites

The code is complete as a portable, simulator-backed ESP-IDF software baseline. It is not a claim of DALI electrical conformance or field commissioning readiness. Before deployment, select and validate the isolated DALI transceiver, implement or validate its RMT/GPIO adapter, calibrate bus and sensor inputs, test flash endurance and power-loss recovery, connect the C6 UART peer, and execute EMC, environmental, FAT, SAT, and site acceptance testing.
