#include <cstdlib>

#include "dali/diagnostics/diagnostics.h"
#include "test_support.h"

int main() {
    dali::diagnostics::DiagnosticEngine engine;
    dali::diagnostics::DiagnosticObservation lamp{};
    lamp.commandedOn = true;
    lamp.lampFailureFlag = true;
    const auto failure = engine.evaluate(lamp);
    CHECK_TRUE(failure.confirmed);
    CHECK_EQ(failure.eventType, dali::LightEventType::LampFailure);

    dali::diagnostics::DiagnosticObservation thermal{};
    thermal.temperatureC = 85.0F;
    thermal.temperatureWarningC = 80.0F;
    thermal.temperatureCriticalC = 90.0F;
    const auto warning = engine.evaluate(thermal);
    CHECK_TRUE(warning.warning);
    CHECK_EQ(warning.eventType, dali::LightEventType::ThermalWarning);
    return EXIT_SUCCESS;
}
