#include "dali/diagnostics/diagnostics.h"

namespace dali::diagnostics {

DiagnosticResult DiagnosticEngine::evaluate(
    const DiagnosticObservation& observation) const noexcept {
    if (observation.lampFailureFlag) {
        return {true, false, true, LightEventType::LampFailure,
                EventSeverity::Critical, 1.0F};
    }
    if (observation.gearFailureFlag) {
        return {true, false, true, LightEventType::ControlGearFailure,
                EventSeverity::Critical, 1.0F};
    }
    if (!observation.present) {
        return {true, false, true, LightEventType::DeviceMissing,
                EventSeverity::Critical, 1.0F};
    }
    if (observation.temperatureC >= observation.temperatureCriticalC) {
        return {true, true, true, LightEventType::ThermalCritical,
                EventSeverity::Critical, 1.0F};
    }
    if (observation.temperatureC >= observation.temperatureWarningC) {
        return {false, true, false, LightEventType::ThermalWarning,
                EventSeverity::Warning, 0.8F};
    }
    if (observation.currentAbnormal || observation.powerAbnormal) {
        return {false, true, false, LightEventType::ElectricalDrift,
                EventSeverity::Warning, 0.5F};
    }
    return {};
}

}  // namespace dali::diagnostics
