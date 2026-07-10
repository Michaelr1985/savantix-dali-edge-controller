# Phase 1 Foundation Verification

**Date:** 2026-07-10  
**Target:** ESP32-S3  
**ESP-IDF:** v6.0.2  
**Branch:** `codex/dali-edge-controller`

Host tools: CMake 4.3.3 and Apple Clang 12.0.0. ESP-IDF used its installed GNU 15.2.0 ESP32-S3 cross-compiler and Python 3.12.13 environment.

## Red-green evidence

| Behavior | Observed red state | Observed green state |
| --- | --- | --- |
| Runtime configuration | Host compile failed because `app_config.h` did not exist | Contract executable returned 0; ESP-IDF build accepted generated Kconfig values |
| Domain types | Host compile failed because `dali/core/types.h` did not exist | Address bounds and unsupported/valid reading tests passed |
| PHY abstraction | Host compile failed because `dali/phy/i_dali_phy.h` did not exist | Abstract-interface contract test passed |
| Simulated PHY | Both host tests failed because the simulator header did not exist | Behavior and 32-entry capacity tests passed |
| Time abstraction | Host compile failed because `dali/time/i_time_source.h` did not exist | Monotonic and Unix-time validity contract passed |
| Boot composition | Host compile failed because `foundation_runtime.h` did not exist | Invalid-config rejection and healthy-PHY startup passed |

The initial ESP-IDF build also detected an incorrectly positioned history-partition size field. The CSV was corrected from an empty size to a blank offset plus `1M` size, and the same build then passed. Moving `Kconfig.projbuild` into the `main` component was required for ESP-IDF to generate Savantix configuration symbols; a clean reconfigure verified them.

## Host verification

Commands:

```bash
cmake -S test/host -B build/host -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
```

Expected test inventory:

1. `domain_types`
2. `phy_contract`
3. `test_simulated_dali_phy`
4. `test_simulated_dali_phy_capacity`
5. `time_contract`
6. `foundation_composition`

Recorded result before documentation: 6 of 6 passed, 0 failed.

## ESP-IDF verification

Commands:

```bash
. "$HOME/esp/esp-idf-v6.0.2/export.sh"
idf.py --version
idf.py set-target esp32s3
idf.py build
```

Recorded result:

- Version: `ESP-IDF v6.0.2`
- Exit status: 0
- Firmware: `build/idf/savantix_dali_edge_controller.bin`
- Firmware size: `0x28b40` bytes
- Smallest app partition: `0x200000` bytes
- Free app-partition space: `0x1d74c0` bytes (92%)
- Bootloader size: `0x5240` bytes with 36% bootloader partition free

The SDK printed non-fatal configuration notifications originating in upstream ESP-IDF component defaults. There were no Savantix compiler warnings because project components compile with `-Wall -Wextra -Werror`.

## Verified requirements

- Native ESP-IDF application for ESP32-S3.
- C++17 compilation.
- Central Kconfig configuration with safe disabled GPIO defaults.
- Four-megabyte flash declaration and valid NVS/app/history partition layout.
- Explicit unsupported, temporarily unavailable, invalid, estimated, and valid measurement quality.
- DALI short-address range 0-63 and group range 0-15.
- Replaceable physical-layer contract.
- Bounded, deterministic simulator with no sleeps or heap allocation.
- Monotonic time independent of wall-clock availability.
- Configuration validation before PHY startup.
- Host source tests and ESP-IDF cross-build.

## Physical validation still required

- ESP32-S3 board flash and boot-log capture.
- Isolated DALI transceiver selection and schematic review.
- DALI voltage, current, polarity, timing, collision, stuck-line, and isolation tests.
- GPIO polarity and ADC calibration.
- Task stack high-water measurements after runtime tasks exist.
- Brownout, watchdog, EMC, thermal, vibration, ingress, and underground site testing.

These items are not software failures; they remain explicitly unverified deployment prerequisites.
