#pragma once

#include <cstdint>

#include "dali/core/types.h"
#include "dali/phy/i_dali_phy.h"

namespace dali::protocol {

struct BackwardResponse final {
    std::uint8_t value{0};
    ReadingQuality quality{ReadingQuality::Unknown};
};

struct BasicStatus final {
    bool controlGearFailure{false};
    bool lampFailure{false};
    bool lampPowerOn{false};
    bool limitError{false};
    bool fadeRunning{false};
    bool resetState{false};
    bool missingShortAddress{false};
    bool powerFailure{false};
};

[[nodiscard]] BackwardResponse makeBackwardResponse(
    PhyResult result,
    std::uint8_t value) noexcept;
[[nodiscard]] Reading<bool> parseYesNo(
    const BackwardResponse& response) noexcept;
[[nodiscard]] Reading<BasicStatus> parseBasicStatus(
    const BackwardResponse& response) noexcept;

}  // namespace dali::protocol
