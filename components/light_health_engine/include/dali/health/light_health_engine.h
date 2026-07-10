#pragma once

#include <cstdint>

#include "dali/core/types.h"

namespace dali::health {

class FaultPersistence final {
public:
    FaultPersistence(std::uint8_t warningSamples,
                     std::uint8_t faultSamples,
                     std::uint8_t recoverySamples) noexcept
        : warningSamples_{warningSamples},
          faultSamples_{faultSamples},
          recoverySamples_{recoverySamples} {}

    bool observe(bool abnormal) noexcept;
    [[nodiscard]] bool warningActive() const noexcept { return warningActive_; }
    [[nodiscard]] bool faultActive() const noexcept { return faultActive_; }
    [[nodiscard]] std::uint8_t abnormalCount() const noexcept { return abnormalCount_; }

private:
    std::uint8_t warningSamples_;
    std::uint8_t faultSamples_;
    std::uint8_t recoverySamples_;
    std::uint8_t abnormalCount_{0};
    std::uint8_t consecutiveAbnormal_{0};
    std::uint8_t consecutiveHealthy_{0};
    bool warningActive_{false};
    bool faultActive_{false};
};

struct HealthObservation final {
    bool present{true};
    bool commandedOn{false};
    bool lampFailure{false};
    bool gearFailure{false};
    bool thermalWarning{false};
    bool thermalCritical{false};
    bool currentDrift{false};
    bool powerDrift{false};
    bool intermittentFault{false};
    bool poorCommunication{false};
};

struct HealthScoringConfig final {
    float confirmedLampFailure{80.0F};
    float confirmedGearFailure{80.0F};
    float deviceMissing{70.0F};
    float criticalTemperature{60.0F};
    float intermittentFailure{40.0F};
    float currentDeviation{25.0F};
    float powerDeviation{25.0F};
    float thermalWarning{20.0F};
    float poorCommunication{15.0F};
    float recoveryStep{1.0F};
};

struct HealthEvaluation final {
    float healthScore{100.0F};
    LightState state{LightState::Unknown};
    std::uint32_t activeFaultFlags{0};
};

class LightHealthEngine final {
public:
    LightHealthEngine(HealthScoringConfig config = {}) noexcept
        : config_{config} {}

    [[nodiscard]] HealthEvaluation evaluate(
        const HealthObservation& observation) noexcept;
    [[nodiscard]] float score() const noexcept { return score_; }

private:
    HealthScoringConfig config_{};
    float score_{100.0F};
};

}  // namespace dali::health
