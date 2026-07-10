#pragma once

#include <cstdint>

#include "dali/core/types.h"

namespace dali::diagnostics {

struct DiagnosticObservation final {
    bool present{true};
    bool commandedOn{false};
    bool lampFailureFlag{false};
    bool gearFailureFlag{false};
    float temperatureC{0.0F};
    float temperatureWarningC{80.0F};
    float temperatureCriticalC{90.0F};
    bool currentAbnormal{false};
    bool powerAbnormal{false};
};

struct DiagnosticResult final {
    bool confirmed{false};
    bool warning{false};
    bool critical{false};
    LightEventType eventType{LightEventType::HealthScoreChanged};
    EventSeverity severity{EventSeverity::Info};
    float confidence{0.0F};
};

class DiagnosticEngine final {
public:
    [[nodiscard]] DiagnosticResult evaluate(
        const DiagnosticObservation& observation) const noexcept;
};

}  // namespace dali::diagnostics
