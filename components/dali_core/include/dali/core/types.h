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

    [[nodiscard]] constexpr std::uint8_t value() const noexcept {
        return value_;
    }

    friend constexpr bool operator==(ShortAddress lhs,
                                     ShortAddress rhs) noexcept {
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

    [[nodiscard]] constexpr std::uint8_t value() const noexcept {
        return value_;
    }

    friend constexpr bool operator==(GroupAddress lhs,
                                     GroupAddress rhs) noexcept {
        return lhs.value_ == rhs.value_;
    }

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

enum class EventSeverity : std::uint8_t {
    Info,
    Warning,
    Critical,
};

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
        T value,
        std::uint64_t uptimeMs,
        std::uint64_t unixTimeSeconds,
        bool hasUnixTime = false) noexcept {
        return {value, ReadingQuality::Valid, uptimeMs, unixTimeSeconds,
                hasUnixTime};
    }

    [[nodiscard]] static constexpr Reading estimated(
        T value,
        std::uint64_t uptimeMs) noexcept {
        return {value, ReadingQuality::Estimated, uptimeMs, 0, false};
    }

    [[nodiscard]] static constexpr Reading temporarilyUnavailable() noexcept {
        return {{}, ReadingQuality::TemporarilyUnavailable, 0, 0, false};
    }

    [[nodiscard]] static constexpr Reading unsupported() noexcept {
        return {{}, ReadingQuality::Unsupported, 0, 0, false};
    }

    [[nodiscard]] static constexpr Reading invalid() noexcept {
        return {{}, ReadingQuality::Invalid, 0, 0, false};
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

struct LightEvent final {
    std::uint32_t eventId{0};
    std::uint8_t shortAddress{0};
    LightEventType type{LightEventType::HealthScoreChanged};
    EventSeverity severity{EventSeverity::Info};
    std::uint64_t timestamp{0};
    float healthScore{100.0F};
    std::uint32_t activeFaultFlags{0};
    LightRealtimeData snapshot{};
    char description[160]{};
};

}  // namespace dali
