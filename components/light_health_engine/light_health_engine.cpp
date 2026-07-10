#include "dali/health/light_health_engine.h"

#include <algorithm>

namespace dali::health {

bool FaultPersistence::observe(bool abnormal) noexcept {
    const bool oldWarning = warningActive_;
    if (abnormal) {
        if (abnormalCount_ < 255U) ++abnormalCount_;
        if (consecutiveAbnormal_ < 255U) ++consecutiveAbnormal_;
        consecutiveHealthy_ = 0;
    } else {
        abnormalCount_ = 0;
        consecutiveAbnormal_ = 0;
        if (consecutiveHealthy_ < 255U) ++consecutiveHealthy_;
        if (consecutiveHealthy_ >= recoverySamples_) {
            warningActive_ = false;
            faultActive_ = false;
        }
    }
    if (abnormalCount_ >= warningSamples_) warningActive_ = true;
    if (consecutiveAbnormal_ >= faultSamples_) faultActive_ = true;
    return oldWarning != warningActive_;
}

HealthEvaluation LightHealthEngine::evaluate(
    const HealthObservation& observation) noexcept {
    float deductions = 0.0F;
    std::uint32_t flags = 0;
    if (observation.lampFailure) {
        deductions += config_.confirmedLampFailure;
        flags |= 1U << 0U;
    }
    if (observation.gearFailure) {
        deductions += config_.confirmedGearFailure;
        flags |= 1U << 1U;
    }
    if (!observation.present) {
        deductions += config_.deviceMissing;
        flags |= 1U << 2U;
    }
    if (observation.thermalCritical) {
        deductions += config_.criticalTemperature;
        flags |= 1U << 3U;
    } else if (observation.thermalWarning) {
        deductions += config_.thermalWarning;
        flags |= 1U << 4U;
    }
    if (observation.intermittentFault) deductions += config_.intermittentFailure;
    if (observation.currentDrift) deductions += config_.currentDeviation;
    if (observation.powerDrift) deductions += config_.powerDeviation;
    if (observation.poorCommunication) deductions += config_.poorCommunication;

    const float target = std::max(0.0F, 100.0F - deductions);
    if (score_ > target) {
        score_ = std::max(target, score_ - deductions);
    } else if (score_ < 100.0F && deductions == 0.0F) {
        score_ = std::min(100.0F, score_ + config_.recoveryStep);
    }

    LightState state = LightState::OnHealthy;
    if (!observation.present) state = LightState::Missing;
    else if (observation.lampFailure || observation.gearFailure) state = LightState::Failed;
    else if (observation.thermalCritical || score_ < 20.0F) state = LightState::Failed;
    else if (score_ < 50.0F) state = LightState::ServiceRequired;
    else if (deductions > 0.0F) state = LightState::Warning;
    else if (!observation.commandedOn) state = LightState::OffCommanded;
    return {score_, state, flags};
}

}  // namespace dali::health
