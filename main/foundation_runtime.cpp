#include "foundation_runtime.h"

FoundationResult initialiseFoundation(
    const SmartLightConfig& config,
    dali::IDaliPhy& phy) noexcept {
    if (!config.validate()) {
        return FoundationResult::InvalidConfiguration;
    }
    if (phy.init() != dali::PhyResult::Ok) {
        return FoundationResult::PhyInitialisationFailed;
    }
    const dali::PhyHealth health = phy.health();
    if (!health.initialised || !health.busPowered ||
        !health.transceiverHealthy || health.lineStuckHigh ||
        health.lineStuckLow) {
        return FoundationResult::PhyUnhealthy;
    }
    return FoundationResult::Ready;
}
