#pragma once

#include <cstdint>

#include "app_config.h"
#include "dali/phy/i_dali_phy.h"

enum class FoundationResult : std::uint8_t {
    Ready,
    InvalidConfiguration,
    PhyInitialisationFailed,
    PhyUnhealthy,
};

[[nodiscard]] FoundationResult initialiseFoundation(
    const SmartLightConfig& config,
    dali::IDaliPhy& phy) noexcept;
