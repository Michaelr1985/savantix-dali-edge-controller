# Savantix DALI Phase 1 Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Create a compiling ESP-IDF v6.0.2 ESP32-S3 application with central configuration, portable domain types, monotonic time abstraction, and a bounded deterministic simulated DALI physical layer proven by host tests.

**Architecture:** The root is a native ESP-IDF application. `dali_core`, `dali_phy`, `dali_simulator`, and `time_service` are focused components. Portable source files are also compiled directly by a standalone host CMake test project so behavior can be tested without ESP32 hardware.

**Tech Stack:** C++17, ESP-IDF v6.0.2, ESP32-S3, CMake, CTest, FreeRTOS, ESP logging

---

## File map

| File | Responsibility |
| --- | --- |
| `CMakeLists.txt` | ESP-IDF project entry |
| `Kconfig.projbuild` | User-facing configuration options |
| `sdkconfig.defaults` | ESP32-S3 development defaults |
| `partitions.csv` | App, NVS, and history storage layout |
| `main/CMakeLists.txt` | Main component dependencies |
| `main/app_main.cpp` | Boot composition and demo heartbeat |
| `main/app_config.h/.cpp` | Validated runtime configuration snapshot |
| `components/dali_core/include/dali/core/types.h` | Strong domain types and validity-bearing readings |
| `components/dali_phy/include/dali/phy/i_dali_phy.h` | Hardware-independent DALI PHY contract |
| `components/dali_simulator/include/dali/sim/simulated_dali_phy.h` | Bounded deterministic simulator API |
| `components/dali_simulator/simulated_dali_phy.cpp` | Simulator behavior |
| `components/time_service/include/dali/time/i_time_source.h` | Time contract |
| `components/time_service/include/dali/time/steady_time_source.h` | ESP monotonic-time adapter |
| `components/time_service/steady_time_source.cpp` | ESP timer implementation |
| `test/host/CMakeLists.txt` | Portable test build |
| `test/host/test_support.h` | Minimal assertion/test runner helpers |
| `test/host/test_domain_types.cpp` | Domain-type tests |
| `test/host/test_simulated_dali_phy.cpp` | Simulator contract tests |
| `README.md` | Build, test, safety boundary, and Phase 1 usage |

### Task 1: Establish the native ESP-IDF project shell

**Files:**
- Create: `CMakeLists.txt`
- Create: `sdkconfig.defaults`
- Create: `partitions.csv`
- Create: `main/CMakeLists.txt`
- Create: `main/app_main.cpp`

- [ ] **Step 1: Create the smallest ESP-IDF source shell**

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(savantix_dali_edge_controller)
```

Create `sdkconfig.defaults`:

```ini
CONFIG_IDF_TARGET="esp32s3"
CONFIG_COMPILER_CXX_EXCEPTIONS=n
CONFIG_COMPILER_CXX_RTTI=n
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_ESP_MAIN_TASK_STACK_SIZE=6144
CONFIG_FREERTOS_HZ=1000
```

Create `partitions.csv`:

```csv
# Name,      Type, SubType, Offset,  Size,    Flags
nvs,         data, nvs,     0x9000,  0x6000,
phy_init,    data, phy,     0xf000,  0x1000,
factory,     app,  factory, 0x10000, 2M,
dali_history,data, 0x40,             1M,
```

Create `main/CMakeLists.txt`:

```cmake
idf_component_register(
    SRCS "app_main.cpp"
    INCLUDE_DIRS "."
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
```

Create `main/app_main.cpp`:

```cpp
#include "esp_log.h"

extern "C" void app_main() {
    ESP_LOGI("savantix", "DALI edge controller foundation booted");
}
```

- [ ] **Step 2: Run the ESP-IDF build and record the environmental result**

Run:

```bash
idf.py set-target esp32s3
idf.py build
```

Expected after ESP-IDF v6.0.2 is installed and exported: exit 0 and an application binary for `esp32s3`. If `idf.py` is unavailable, install/export v6.0.2 before continuing; do not count an unavailable toolchain as a passing build.

- [ ] **Step 3: Commit the shell**

```bash
git add CMakeLists.txt sdkconfig.defaults partitions.csv main/CMakeLists.txt main/app_main.cpp
git commit -m "build: scaffold ESP-IDF DALI controller"
```

### Task 2: Add central Kconfig-backed runtime configuration

**Files:**
- Create: `Kconfig.projbuild`
- Create: `main/app_config.h`
- Create: `main/app_config.cpp`
- Modify: `main/CMakeLists.txt`
- Modify: `main/app_main.cpp`

- [ ] **Step 1: Write a compile-time contract test as a temporary host translation unit**

Create `test/host/test_app_config_compile.cpp`:

```cpp
#include "app_config.h"

static_assert(SmartLightConfig::kMaximumDaliLights == 64);

int main() {
    const SmartLightConfig config = SmartLightConfig::fromKconfig();
    return config.validate() ? 0 : 1;
}
```

- [ ] **Step 2: Verify the contract test fails because configuration does not exist**

Run:

```bash
c++ -std=c++17 -Imain test/host/test_app_config_compile.cpp -c
```

Expected: failure containing `app_config.h: No such file or directory`.

- [ ] **Step 3: Define configuration options and the validated snapshot**

Create `Kconfig.projbuild`:

```kconfig
menu "Savantix DALI Edge Controller"

config SAVANTIX_DALI_TX_GPIO
    int "DALI transceiver TX GPIO"
    default -1

config SAVANTIX_DALI_RX_GPIO
    int "DALI transceiver RX GPIO"
    default -1

config SAVANTIX_BUS_POWER_GPIO
    int "DALI bus-power status GPIO"
    default -1

config SAVANTIX_TRANSCEIVER_FAULT_GPIO
    int "DALI transceiver fault GPIO"
    default -1

config SAVANTIX_BUS_VOLTAGE_ADC_GPIO
    int "Optional bus-voltage ADC GPIO"
    default -1

config SAVANTIX_BUS_CURRENT_ADC_GPIO
    int "Optional bus-current ADC GPIO"
    default -1

config SAVANTIX_C6_TX_GPIO
    int "ESP32-C6 UART TX GPIO"
    default -1

config SAVANTIX_C6_RX_GPIO
    int "ESP32-C6 UART RX GPIO"
    default -1

config SAVANTIX_C6_RTS_GPIO
    int "Optional ESP32-C6 UART RTS GPIO"
    default -1

config SAVANTIX_C6_CTS_GPIO
    int "Optional ESP32-C6 UART CTS GPIO"
    default -1

config SAVANTIX_MAX_LIGHTS
    int "Maximum DALI lights"
    range 1 64
    default 64

config SAVANTIX_ON_STATUS_POLL_MS
    int "Commanded-on status poll period (ms)"
    range 100 3600000
    default 5000

config SAVANTIX_OFF_STATUS_POLL_MS
    int "Commanded-off status poll period (ms)"
    range 100 3600000
    default 30000

config SAVANTIX_WARNING_STATUS_POLL_MS
    int "Warning status poll period (ms)"
    range 100 60000
    default 2000

config SAVANTIX_MAX_COMMAND_RETRIES
    int "Maximum command retries"
    range 0 8
    default 3

config SAVANTIX_NORMAL_BUS_UTILISATION_PERCENT
    int "Maximum normal bus utilisation (%)"
    range 1 100
    default 30

config SAVANTIX_DIAGNOSTIC_BUS_UTILISATION_PERCENT
    int "Maximum diagnostic bus utilisation (%)"
    range 1 100
    default 60

config SAVANTIX_TEMPERATURE_WARNING_C
    int "Temperature warning (degrees C)"
    range 0 200
    default 80

config SAVANTIX_TEMPERATURE_CRITICAL_C
    int "Temperature critical (degrees C)"
    range 1 220
    default 90

config SAVANTIX_WARNING_PERSISTENCE_SAMPLES
    int "Warning abnormal samples"
    range 1 32
    default 3

config SAVANTIX_FAULT_PERSISTENCE_SAMPLES
    int "Fault consecutive samples"
    range 1 32
    default 5

config SAVANTIX_RECOVERY_PERSISTENCE_SAMPLES
    int "Recovery healthy samples"
    range 1 64
    default 10

config SAVANTIX_C6_ENABLE
    bool "Enable ESP32-C6 interface"
    default n

config SAVANTIX_DEMO_ENABLE
    bool "Enable simulated DALI demo"
    default y

config SAVANTIX_ANOMALY_INTERFACE_ENABLE
    bool "Enable anomaly-model interface"
    default n

endmenu
```

Create `main/app_config.h`:

```cpp
#pragma once

#include <cstdint>

struct SmartLightConfig final {
    static constexpr std::uint8_t kMaximumDaliLights = 64;

    int daliTxGpio{-1};
    int daliRxGpio{-1};
    int busPowerGpio{-1};
    int transceiverFaultGpio{-1};
    int busVoltageAdcGpio{-1};
    int busCurrentAdcGpio{-1};
    int c6TxGpio{-1};
    int c6RxGpio{-1};
    int c6RtsGpio{-1};
    int c6CtsGpio{-1};
    std::uint8_t maximumLights{64};
    std::uint32_t onStatusPollMs{5000};
    std::uint32_t offStatusPollMs{30000};
    std::uint32_t warningStatusPollMs{2000};
    std::uint8_t maxCommandRetries{3};
    float maximumNormalBusUtilisation{0.30F};
    float maximumDiagnosticBusUtilisation{0.60F};
    float temperatureWarningC{80.0F};
    float temperatureCriticalC{90.0F};
    std::uint8_t warningPersistenceSamples{3};
    std::uint8_t faultPersistenceSamples{5};
    std::uint8_t recoveryPersistenceSamples{10};
    bool c6Enabled{false};
    bool demoEnabled{true};
    bool anomalyInterfaceEnabled{false};

    [[nodiscard]] static constexpr SmartLightConfig defaults() noexcept {
        return {};
    }
    [[nodiscard]] static SmartLightConfig fromKconfig() noexcept;
    [[nodiscard]] bool validate() const noexcept;
};
```

Create `main/app_config.cpp`. Under `ESP_PLATFORM`, include `sdkconfig.h` and map every `CONFIG_SAVANTIX_*` option above. Outside ESP-IDF, return `defaults()` so host tests compile without generated Kconfig headers. Implement `validate()` with these exact rules:

```cpp
#include "app_config.h"

#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#endif

SmartLightConfig SmartLightConfig::fromKconfig() noexcept {
#ifdef ESP_PLATFORM
    SmartLightConfig config{};
    config.daliTxGpio = CONFIG_SAVANTIX_DALI_TX_GPIO;
    config.daliRxGpio = CONFIG_SAVANTIX_DALI_RX_GPIO;
    config.busPowerGpio = CONFIG_SAVANTIX_BUS_POWER_GPIO;
    config.transceiverFaultGpio = CONFIG_SAVANTIX_TRANSCEIVER_FAULT_GPIO;
    config.busVoltageAdcGpio = CONFIG_SAVANTIX_BUS_VOLTAGE_ADC_GPIO;
    config.busCurrentAdcGpio = CONFIG_SAVANTIX_BUS_CURRENT_ADC_GPIO;
    config.c6TxGpio = CONFIG_SAVANTIX_C6_TX_GPIO;
    config.c6RxGpio = CONFIG_SAVANTIX_C6_RX_GPIO;
    config.c6RtsGpio = CONFIG_SAVANTIX_C6_RTS_GPIO;
    config.c6CtsGpio = CONFIG_SAVANTIX_C6_CTS_GPIO;
    config.maximumLights = CONFIG_SAVANTIX_MAX_LIGHTS;
    config.onStatusPollMs = CONFIG_SAVANTIX_ON_STATUS_POLL_MS;
    config.offStatusPollMs = CONFIG_SAVANTIX_OFF_STATUS_POLL_MS;
    config.warningStatusPollMs = CONFIG_SAVANTIX_WARNING_STATUS_POLL_MS;
    config.maxCommandRetries = CONFIG_SAVANTIX_MAX_COMMAND_RETRIES;
    config.maximumNormalBusUtilisation =
        static_cast<float>(CONFIG_SAVANTIX_NORMAL_BUS_UTILISATION_PERCENT) / 100.0F;
    config.maximumDiagnosticBusUtilisation =
        static_cast<float>(CONFIG_SAVANTIX_DIAGNOSTIC_BUS_UTILISATION_PERCENT) / 100.0F;
    config.temperatureWarningC = CONFIG_SAVANTIX_TEMPERATURE_WARNING_C;
    config.temperatureCriticalC = CONFIG_SAVANTIX_TEMPERATURE_CRITICAL_C;
    config.warningPersistenceSamples = CONFIG_SAVANTIX_WARNING_PERSISTENCE_SAMPLES;
    config.faultPersistenceSamples = CONFIG_SAVANTIX_FAULT_PERSISTENCE_SAMPLES;
    config.recoveryPersistenceSamples = CONFIG_SAVANTIX_RECOVERY_PERSISTENCE_SAMPLES;
    config.c6Enabled = CONFIG_SAVANTIX_C6_ENABLE;
    config.demoEnabled = CONFIG_SAVANTIX_DEMO_ENABLE;
    config.anomalyInterfaceEnabled = CONFIG_SAVANTIX_ANOMALY_INTERFACE_ENABLE;
    return config;
#else
    return defaults();
#endif
}

bool SmartLightConfig::validate() const noexcept {
    const bool lightCountValid = maximumLights > 0 &&
                                 maximumLights <= kMaximumDaliLights;
    const bool periodsValid = onStatusPollMs > 0 && offStatusPollMs > 0 &&
                              warningStatusPollMs > 0;
    const bool retryValid = maxCommandRetries <= 8;
    const bool utilisationValid = maximumNormalBusUtilisation > 0.0F &&
        maximumNormalBusUtilisation <= 1.0F &&
        maximumDiagnosticBusUtilisation >= maximumNormalBusUtilisation &&
        maximumDiagnosticBusUtilisation <= 1.0F;
    const bool temperatureValid = temperatureCriticalC > temperatureWarningC;
    const bool persistenceValid = warningPersistenceSamples > 0 &&
        faultPersistenceSamples > 0 && recoveryPersistenceSamples > 0;
    const bool daliPinsDistinct = daliTxGpio < 0 || daliRxGpio < 0 ||
                                  daliTxGpio != daliRxGpio;
    const bool c6PinsValid = !c6Enabled ||
        (c6TxGpio >= 0 && c6RxGpio >= 0 && c6TxGpio != c6RxGpio);
    return lightCountValid && periodsValid && retryValid && utilisationValid &&
           temperatureValid && persistenceValid && daliPinsDistinct &&
           c6PinsValid;
}
```

- [ ] **Step 4: Compile the configuration contract and verify it passes**

Run:

```bash
c++ -std=c++17 -Imain test/host/test_app_config_compile.cpp main/app_config.cpp -o build_app_config_contract
./build_app_config_contract
```

Expected: exit 0. Remove `build_app_config_contract` after verification; Task 3 replaces this temporary check with the permanent host suite.

- [ ] **Step 5: Wire validated configuration into boot**

Add `app_config.cpp` to `main/CMakeLists.txt`. In `app_main`, construct `SmartLightConfig::fromKconfig()`, log an error and return when validation fails, and log the maximum light count on success.

- [ ] **Step 6: Rebuild and commit**

```bash
idf.py build
git add Kconfig.projbuild main/app_config.h main/app_config.cpp main/CMakeLists.txt main/app_main.cpp test/host/test_app_config_compile.cpp
git commit -m "feat: add validated controller configuration"
```

Expected: build exit 0.

### Task 3: Define portable domain types with explicit validity

**Files:**
- Create: `components/dali_core/CMakeLists.txt`
- Create: `components/dali_core/include/dali/core/types.h`
- Create: `test/host/CMakeLists.txt`
- Create: `test/host/test_support.h`
- Create: `test/host/test_domain_types.cpp`
- Delete: `test/host/test_app_config_compile.cpp`

- [ ] **Step 1: Create the permanent host test harness and failing domain tests**

Create `test/host/test_support.h`:

```cpp
#pragma once

#include <cstdlib>
#include <iostream>

#define CHECK_TRUE(expression) do { \
    if (!(expression)) { \
        std::cerr << __FILE__ << ':' << __LINE__ \
                  << " CHECK_TRUE failed: " #expression << '\n'; \
        return EXIT_FAILURE; \
    } \
} while (false)

#define CHECK_EQ(actual, expected) CHECK_TRUE((actual) == (expected))
```

Create `test/host/test_domain_types.cpp`:

```cpp
#include <cstdlib>
#include "dali/core/types.h"
#include "test_support.h"

int main() {
    using namespace dali;
    const auto address = ShortAddress::fromRaw(63);
    CHECK_TRUE(address.has_value());
    CHECK_EQ(address->value(), 63U);
    CHECK_TRUE(!ShortAddress::fromRaw(64).has_value());

    const Reading<float> unsupported = Reading<float>::unsupported();
    CHECK_TRUE(!unsupported.hasValue());
    CHECK_EQ(unsupported.quality, ReadingQuality::Unsupported);

    const Reading<float> measured = Reading<float>::valid(230.5F, 1000U, 0U);
    CHECK_TRUE(measured.hasValue());
    CHECK_EQ(measured.value, 230.5F);
    CHECK_EQ(measured.uptimeMs, 1000U);
    return EXIT_SUCCESS;
}
```

Create `test/host/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(savantix_dali_host_tests LANGUAGES CXX)
enable_testing()

add_executable(test_domain_types test_domain_types.cpp)
target_include_directories(test_domain_types PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
)
target_compile_features(test_domain_types PRIVATE cxx_std_17)
target_compile_options(test_domain_types PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME domain_types COMMAND test_domain_types)
```

- [ ] **Step 2: Verify the test fails for the missing production header**

```bash
cmake -S test/host -B build/host
cmake --build build/host
```

Expected: compilation failure containing `dali/core/types.h: No such file or directory`.

- [ ] **Step 3: Implement strong addresses, reading validity, and core records**

Create `components/dali_core/include/dali/core/types.h`:

```cpp
#pragma once

#include <cstdint>
#include <optional>

namespace dali {

class ShortAddress final {
public:
    [[nodiscard]] static constexpr std::optional<ShortAddress>
    fromRaw(std::uint8_t value) noexcept {
        return value <= 63U ? std::optional<ShortAddress>{ShortAddress{value}}
                            : std::nullopt;
    }
    [[nodiscard]] constexpr std::uint8_t value() const noexcept { return value_; }
    friend constexpr bool operator==(ShortAddress lhs, ShortAddress rhs) noexcept {
        return lhs.value_ == rhs.value_;
    }
private:
    explicit constexpr ShortAddress(std::uint8_t value) noexcept : value_{value} {}
    std::uint8_t value_;
};

class GroupAddress final {
public:
    [[nodiscard]] static constexpr std::optional<GroupAddress>
    fromRaw(std::uint8_t value) noexcept {
        return value <= 15U ? std::optional<GroupAddress>{GroupAddress{value}}
                            : std::nullopt;
    }
    [[nodiscard]] constexpr std::uint8_t value() const noexcept { return value_; }
private:
    explicit constexpr GroupAddress(std::uint8_t value) noexcept : value_{value} {}
    std::uint8_t value_;
};

enum class ReadingQuality : std::uint8_t {
    Unknown,
    Valid,
    Estimated,
    TemporarilyUnavailable,
    Unsupported,
    Invalid,
};

enum class LightState : std::uint8_t {
    Unknown,
    Learning,
    OffCommanded,
    OnHealthy,
    DimmedHealthy,
    Warning,
    ServiceRequired,
    Failed,
    Missing,
    CommunicationFault,
    Recovering,
};

enum class EventSeverity : std::uint8_t { Info, Warning, Critical };

enum class LightEventType : std::uint8_t {
    DeviceDiscovered,
    DeviceReplaced,
    DeviceMissing,
    DeviceRecovered,
    LampFailure,
    ControlGearFailure,
    PossibleLampFailure,
    DaliBusFailure,
    DaliBusRecovered,
    CommunicationDegraded,
    CommunicationRecovered,
    ThermalWarning,
    ThermalCritical,
    ElectricalDrift,
    IntermittentFault,
    HealthScoreChanged,
    ServiceRecommended,
    BaselineEstablished,
    BaselineReset,
};

template <typename T>
struct Reading final {
    T value{};
    ReadingQuality quality{ReadingQuality::Unknown};
    std::uint64_t uptimeMs{0};
    std::uint64_t unixTimeSeconds{0};
    bool unixTimeValid{false};

    [[nodiscard]] static constexpr Reading valid(
        T value, std::uint64_t uptimeMs, std::uint64_t unixTimeSeconds,
        bool hasUnixTime = false) noexcept {
        return {value, ReadingQuality::Valid, uptimeMs, unixTimeSeconds,
                hasUnixTime};
    }

    [[nodiscard]] static constexpr Reading temporarilyUnavailable() noexcept {
        return {{}, ReadingQuality::TemporarilyUnavailable, 0, 0, false};
    }

    [[nodiscard]] static constexpr Reading unsupported() noexcept {
        return {{}, ReadingQuality::Unsupported, 0, 0, false};
    }

    [[nodiscard]] constexpr bool hasValue() const noexcept {
        return quality == ReadingQuality::Valid ||
               quality == ReadingQuality::Estimated;
    }
};

struct DaliCapabilities final {
    bool supportsBasicStatus{false};
    bool supportsEnergyData{false};
    bool supportsInputPower{false};
    bool supportsOutputPower{false};
    bool supportsInputVoltage{false};
    bool supportsInputCurrent{false};
    bool supportsOutputCurrent{false};
    bool supportsTemperature{false};
    bool supportsOperatingHours{false};
    bool supportsStartCounter{false};
    bool supportsFailureCounters{false};
    bool supportsDiagnosticData{false};
    bool supportsIdentificationData{false};
};

struct LightRealtimeData final {
    std::uint8_t shortAddress{0};
    bool present{false};
    bool commandedOn{false};
    std::uint8_t commandedLevel{0};
    Reading<std::uint8_t> actualLevel{};
    Reading<bool> lampFailure{};
    Reading<bool> gearFailure{};
    Reading<bool> limitError{};
    Reading<bool> fadeRunning{};
    Reading<bool> resetState{};
    Reading<float> inputVoltage{};
    Reading<float> inputCurrent{};
    Reading<float> inputPower{};
    Reading<float> outputCurrent{};
    Reading<float> outputPower{};
    Reading<float> temperature{};
    std::uint64_t timestampMs{0};
};

struct LightLifetimeData final {
    Reading<std::uint32_t> operatingHours{};
    Reading<std::uint32_t> lightSourceHours{};
    Reading<std::uint32_t> startCount{};
    Reading<std::uint32_t> powerCycleCount{};
    Reading<std::uint32_t> lampFailureCount{};
    Reading<std::uint32_t> gearFailureCount{};
};

struct LightHealthData final {
    float healthScore{100.0F};
    LightState state{LightState::Unknown};
    Reading<float> currentDeviationPercent{};
    Reading<float> powerDeviationPercent{};
    Reading<float> temperatureDeviation{};
    float communicationSuccessRate{0.0F};
    bool deteriorationDetected{false};
    bool intermittentFaultDetected{false};
    std::uint64_t lastHealthyTimestamp{0};
    std::uint64_t lastFaultTimestamp{0};
};

struct LightFeatureVector final {
    Reading<float> commandedLevel{};
    Reading<float> inputVoltage{};
    Reading<float> inputCurrent{};
    Reading<float> inputPower{};
    Reading<float> outputCurrent{};
    Reading<float> outputPower{};
    Reading<float> temperature{};
    Reading<float> currentDeviation{};
    Reading<float> powerDeviation{};
    Reading<float> temperatureDeviation{};
    float communicationErrorRate{0.0F};
    float startFrequency{0.0F};
    float recentFailureFrequency{0.0F};
    Reading<float> sevenDayTrend{};
    Reading<float> thirtyDayTrend{};
};

}  // namespace dali
```

Create `components/dali_core/CMakeLists.txt`:

```cmake
idf_component_register(INCLUDE_DIRS "include")
target_compile_features(${COMPONENT_LIB} INTERFACE cxx_std_17)
```

- [ ] **Step 4: Run the host test and verify green**

```bash
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
```

Expected: `domain_types` passes.

- [ ] **Step 5: Remove the temporary configuration test and commit**

```bash
rm test/host/test_app_config_compile.cpp
git add components/dali_core test/host
git commit -m "feat: add validity-aware DALI domain types"
```

### Task 4: Define the physical-layer contract

**Files:**
- Create: `components/dali_phy/CMakeLists.txt`
- Create: `components/dali_phy/include/dali/phy/i_dali_phy.h`
- Modify: `test/host/CMakeLists.txt`
- Create: `test/host/test_phy_contract.cpp`

- [ ] **Step 1: Write the failing compile-time interface test**

Create `test/host/test_phy_contract.cpp`:

```cpp
#include <type_traits>
#include "dali/phy/i_dali_phy.h"

static_assert(std::has_virtual_destructor_v<dali::IDaliPhy>);
static_assert(std::is_abstract_v<dali::IDaliPhy>);

int main() { return 0; }
```

Add a `test_phy_contract` target and CTest entry to `test/host/CMakeLists.txt` with the PHY include directory.

- [ ] **Step 2: Verify failure for the missing interface**

```bash
cmake -S test/host -B build/host
cmake --build build/host
```

Expected: compilation failure containing `dali/phy/i_dali_phy.h: No such file or directory`.

- [ ] **Step 3: Implement the portable PHY interface**

Create `i_dali_phy.h`:

```cpp
#pragma once

#include <cstdint>

namespace dali {

enum class PhyResult : std::uint8_t {
    Ok,
    Timeout,
    Busy,
    Collision,
    BusPowerLost,
    TransceiverFault,
    InvalidArgument,
    NotInitialised,
    InternalError,
};

struct PhyHealth final {
    bool initialised{false};
    bool busPowered{false};
    bool transceiverHealthy{false};
    bool lineStuckHigh{false};
    bool lineStuckLow{false};
};

class IDaliPhy {
public:
    virtual ~IDaliPhy() = default;
    virtual PhyResult init() noexcept = 0;
    virtual PhyResult transmit(std::uint16_t forwardFrame) noexcept = 0;
    virtual PhyResult receive(std::uint8_t& backwardFrame,
                              std::uint32_t timeoutMs) noexcept = 0;
    [[nodiscard]] virtual bool isBusBusy() const noexcept = 0;
    [[nodiscard]] virtual bool hasCollision() const noexcept = 0;
    [[nodiscard]] virtual PhyHealth health() const noexcept = 0;
    virtual PhyResult reset() noexcept = 0;
};

}  // namespace dali
```

Create the component manifest with the public include directory and dependency on `dali_core`.

- [ ] **Step 4: Verify and commit**

```bash
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
idf.py build
git add components/dali_phy test/host
git commit -m "feat: define isolated DALI PHY contract"
```

Expected: all host tests and the firmware build pass.

### Task 5: Implement a bounded deterministic simulated PHY

**Files:**
- Create: `components/dali_simulator/CMakeLists.txt`
- Create: `components/dali_simulator/include/dali/sim/simulated_dali_phy.h`
- Create: `components/dali_simulator/simulated_dali_phy.cpp`
- Create: `test/host/test_simulated_dali_phy.cpp`
- Modify: `test/host/CMakeLists.txt`

- [ ] **Step 1: Write failing behavior tests**

Create `test/host/test_simulated_dali_phy.cpp` testing these behaviors independently:

```cpp
#include <cstdlib>
#include "dali/sim/simulated_dali_phy.h"
#include "test_support.h"

int main() {
    using namespace dali;
    SimulatedDaliPhy phy;
    std::uint8_t response = 0;

    CHECK_EQ(phy.transmit(0x0190U), PhyResult::NotInitialised);
    CHECK_EQ(phy.init(), PhyResult::Ok);
    CHECK_TRUE(phy.health().busPowered);

    CHECK_EQ(phy.enqueueResponse({PhyResult::Ok, 0xFFU, 3U}), PhyResult::Ok);
    CHECK_EQ(phy.transmit(0x0190U), PhyResult::Ok);
    CHECK_EQ(phy.receive(response, 2U), PhyResult::Timeout);
    CHECK_EQ(phy.receive(response, 3U), PhyResult::Ok);
    CHECK_EQ(response, 0xFFU);

    phy.setBusPowered(false);
    CHECK_EQ(phy.transmit(0x0190U), PhyResult::BusPowerLost);
    CHECK_EQ(phy.reset(), PhyResult::Ok);
    return EXIT_SUCCESS;
}
```

Add a separate capacity test that fills the scripted response queue to `SimulatedDaliPhy::kResponseCapacity`, verifies the next enqueue returns `PhyResult::Busy`, consumes one response, and verifies one enqueue succeeds. Add both executables and tests to the host CMake file, compiling the production simulator source directly.

- [ ] **Step 2: Verify tests fail because the simulator is absent**

```bash
cmake -S test/host -B build/host
cmake --build build/host
```

Expected: compilation failure containing `dali/sim/simulated_dali_phy.h: No such file or directory`.

- [ ] **Step 3: Implement the simulator without dynamic allocation**

Define `SimulatedResponse { PhyResult result; uint8_t value; uint32_t delayMs; }`. Implement `SimulatedDaliPhy` using `std::array<SimulatedResponse, 32>`, head/count indices, a last transmitted frame, bus-power/health/collision/busy controls, and initialisation state. `receive` must leave a queued delayed response in place when `timeoutMs` is too short. `reset` clears transient busy/collision/stuck-line state but does not invent bus power.

The simulator API must expose bounded scripting controls, the last forward frame, transmitted-frame count, and health-input setters. It must not contain sleeps or wall-clock dependencies.

Use this header:

```cpp
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "dali/phy/i_dali_phy.h"

namespace dali {

struct SimulatedResponse final {
    PhyResult result{PhyResult::Timeout};
    std::uint8_t value{0};
    std::uint32_t delayMs{0};
};

class SimulatedDaliPhy final : public IDaliPhy {
public:
    static constexpr std::size_t kResponseCapacity = 32;

    PhyResult init() noexcept override;
    PhyResult transmit(std::uint16_t forwardFrame) noexcept override;
    PhyResult receive(std::uint8_t& backwardFrame,
                      std::uint32_t timeoutMs) noexcept override;
    [[nodiscard]] bool isBusBusy() const noexcept override;
    [[nodiscard]] bool hasCollision() const noexcept override;
    [[nodiscard]] PhyHealth health() const noexcept override;
    PhyResult reset() noexcept override;

    PhyResult enqueueResponse(SimulatedResponse response) noexcept;
    void setBusPowered(bool powered) noexcept;
    void setTransceiverHealthy(bool healthy) noexcept;
    void setBusBusy(bool busy) noexcept;
    void setCollision(bool collision) noexcept;
    void setLineStuckHigh(bool stuck) noexcept;
    void setLineStuckLow(bool stuck) noexcept;
    [[nodiscard]] std::uint16_t lastForwardFrame() const noexcept;
    [[nodiscard]] std::uint32_t transmittedFrameCount() const noexcept;

private:
    std::array<SimulatedResponse, kResponseCapacity> responses_{};
    std::size_t responseHead_{0};
    std::size_t responseCount_{0};
    std::uint16_t lastForwardFrame_{0};
    std::uint32_t transmittedFrameCount_{0};
    PhyHealth health_{false, true, true, false, false};
    bool busBusy_{false};
    bool collision_{false};
};

}  // namespace dali
```

Use this source:

```cpp
#include "dali/sim/simulated_dali_phy.h"

namespace dali {

PhyResult SimulatedDaliPhy::init() noexcept {
    health_.initialised = true;
    return health_.transceiverHealthy ? PhyResult::Ok
                                      : PhyResult::TransceiverFault;
}

PhyResult SimulatedDaliPhy::transmit(std::uint16_t frame) noexcept {
    if (!health_.initialised) return PhyResult::NotInitialised;
    if (!health_.busPowered) return PhyResult::BusPowerLost;
    if (!health_.transceiverHealthy) return PhyResult::TransceiverFault;
    if (busBusy_) return PhyResult::Busy;
    if (collision_) return PhyResult::Collision;
    lastForwardFrame_ = frame;
    ++transmittedFrameCount_;
    return PhyResult::Ok;
}

PhyResult SimulatedDaliPhy::receive(std::uint8_t& frame,
                                    std::uint32_t timeoutMs) noexcept {
    if (!health_.initialised) return PhyResult::NotInitialised;
    if (!health_.busPowered) return PhyResult::BusPowerLost;
    if (responseCount_ == 0) return PhyResult::Timeout;
    const SimulatedResponse& response = responses_[responseHead_];
    if (response.delayMs > timeoutMs) return PhyResult::Timeout;
    const SimulatedResponse consumed = response;
    responseHead_ = (responseHead_ + 1U) % kResponseCapacity;
    --responseCount_;
    if (consumed.result == PhyResult::Ok) frame = consumed.value;
    return consumed.result;
}

bool SimulatedDaliPhy::isBusBusy() const noexcept { return busBusy_; }
bool SimulatedDaliPhy::hasCollision() const noexcept { return collision_; }
PhyHealth SimulatedDaliPhy::health() const noexcept { return health_; }

PhyResult SimulatedDaliPhy::reset() noexcept {
    busBusy_ = false;
    collision_ = false;
    health_.lineStuckHigh = false;
    health_.lineStuckLow = false;
    return health_.transceiverHealthy ? PhyResult::Ok
                                      : PhyResult::TransceiverFault;
}

PhyResult SimulatedDaliPhy::enqueueResponse(SimulatedResponse response) noexcept {
    if (responseCount_ == kResponseCapacity) return PhyResult::Busy;
    const std::size_t tail = (responseHead_ + responseCount_) % kResponseCapacity;
    responses_[tail] = response;
    ++responseCount_;
    return PhyResult::Ok;
}

void SimulatedDaliPhy::setBusPowered(bool value) noexcept {
    health_.busPowered = value;
}
void SimulatedDaliPhy::setTransceiverHealthy(bool value) noexcept {
    health_.transceiverHealthy = value;
}
void SimulatedDaliPhy::setBusBusy(bool value) noexcept { busBusy_ = value; }
void SimulatedDaliPhy::setCollision(bool value) noexcept { collision_ = value; }
void SimulatedDaliPhy::setLineStuckHigh(bool value) noexcept {
    health_.lineStuckHigh = value;
}
void SimulatedDaliPhy::setLineStuckLow(bool value) noexcept {
    health_.lineStuckLow = value;
}
std::uint16_t SimulatedDaliPhy::lastForwardFrame() const noexcept {
    return lastForwardFrame_;
}
std::uint32_t SimulatedDaliPhy::transmittedFrameCount() const noexcept {
    return transmittedFrameCount_;
}

}  // namespace dali
```

- [ ] **Step 4: Verify red-to-green behavior**

```bash
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
idf.py build
```

Expected: domain, PHY contract, simulator behavior, and capacity tests pass; firmware build passes.

- [ ] **Step 5: Commit**

```bash
git add components/dali_simulator test/host
git commit -m "feat: add bounded simulated DALI physical layer"
```

### Task 6: Add monotonic and optional wall-clock time contracts

**Files:**
- Create: `components/time_service/CMakeLists.txt`
- Create: `components/time_service/include/dali/time/i_time_source.h`
- Create: `components/time_service/include/dali/time/esp_time_source.h`
- Create: `components/time_service/esp_time_source.cpp`
- Create: `test/host/test_time_contract.cpp`
- Modify: `test/host/CMakeLists.txt`

- [ ] **Step 1: Write a failing fake-time contract test**

The test defines a local `FakeTimeSource` implementing `ITimeSource`, verifies uptime always exists, verifies Unix time is initially invalid, calls `synchroniseUnixTime(1'800'000'000, 5000)`, advances uptime to 8000, and expects Unix time `1'800'000'003` with validity true.

- [ ] **Step 2: Run and verify missing-interface failure**

```bash
cmake -S test/host -B build/host
cmake --build build/host
```

Expected: compilation failure containing `dali/time/i_time_source.h: No such file or directory`.

- [ ] **Step 3: Implement the time contract and ESP adapter**

`ITimeSource` exposes `uptimeMs()`, `unixTimeValid()`, `unixTimeSeconds()`, and `synchroniseUnixTime(unixSeconds, uptimeAtSyncMs)`. `EspTimeSource` gets uptime from `esp_timer_get_time()`, stores sync state behind a FreeRTOS critical section, and derives Unix time from monotonic elapsed seconds. It never makes control behavior depend on wall-clock validity.

- [ ] **Step 4: Verify and commit**

```bash
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
idf.py build
git add components/time_service test/host
git commit -m "feat: add monotonic controller time abstraction"
```

Expected: all tests and the ESP-IDF build pass.

### Task 7: Compose the Phase 1 demo boot path

**Files:**
- Modify: `main/CMakeLists.txt`
- Modify: `main/app_main.cpp`
- Modify: `main/app_config.cpp`
- Create: `docs/verification/phase-1.md`
- Create: `README.md`

- [ ] **Step 1: Write a failing host composition test**

Create `test/host/test_foundation_composition.cpp` that constructs validated default configuration through a test factory, initialises `SimulatedDaliPhy`, enqueues one response, transmits one frame, receives it, and asserts health and response. This test uses real production classes and no mocks.

- [ ] **Step 2: Verify it fails because the test configuration factory and composition helper do not exist**

```bash
cmake -S test/host -B build/host
cmake --build build/host
```

Expected: a compile failure naming the missing production API.

- [ ] **Step 3: Add the minimal composition API and boot behavior**

Use the existing platform-neutral `SmartLightConfig::defaults()` factory and keep `fromKconfig()` as the ESP adapter. Update `main/CMakeLists.txt` to require `dali_core`, `dali_phy`, `dali_simulator`, and `time_service`. When demo mode is enabled, `app_main` initialises the simulator, logs PHY health, and logs that Phase 1 is ready. It must not start fake polling or claim device discovery before later phases implement those behaviors.

- [ ] **Step 4: Run the complete Phase 1 verification set**

```bash
rm -rf build/host
cmake -S test/host -B build/host -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
idf.py set-target esp32s3
idf.py build
git diff --check
```

Expected: every host test passes, ESP-IDF build exits 0, and `git diff --check` emits no errors.

- [ ] **Step 5: Document exact evidence**

Create `docs/verification/phase-1.md` containing tool versions, commands, exit results, test names, ESP-IDF binary size output, unresolved hardware-only checks, and the commit hash. Create `README.md` with ESP-IDF v6.0.2 setup, target/build/flash/monitor commands, host-test commands, module overview, isolated-PHY warning, GPIO assumptions, current demo behavior, and Phase 1 limitations.

- [ ] **Step 6: Commit the verified phase**

```bash
git add main test/host README.md docs/verification/phase-1.md
git commit -m "feat: complete DALI controller foundation"
```

## Phase 1 completion gate

Do not write the Phase 2 detailed plan until all of these are true:

- [ ] The host suite was observed failing for each introduced behavior before implementation.
- [ ] All host tests pass with warnings treated as errors.
- [ ] ESP-IDF v6.0.2 builds the application for `esp32s3`.
- [ ] Domain types cannot silently treat unsupported data as measurements.
- [ ] Simulator queues are bounded and deterministic.
- [ ] Time-dependent behavior is testable without sleeping.
- [ ] Optional GPIOs default disabled and the README warns against direct DALI-bus connection.
- [ ] Verification evidence is recorded and committed.
