#pragma once

#include <array>
#include <cstdint>

#include "dali/core/types.h"

namespace dali::baseline {

enum class LevelBand : std::uint8_t { Band1, Band2, Band3, Band4, Band5 };

struct BaselineObservation final {
    bool stable{false};
    bool healthy{false};
    bool communicationAcceptable{false};
    Reading<float> inputPower{};
    Reading<float> outputCurrent{};
};

class BaselineStore final {
public:
    [[nodiscard]] static LevelBand bandFor(std::uint8_t commandedLevel) noexcept;
    [[nodiscard]] bool update(std::uint8_t commandedLevel,
                              const BaselineObservation& observation) noexcept;
    [[nodiscard]] float meanInputPower(LevelBand band) const noexcept;
    [[nodiscard]] float meanOutputCurrent(LevelBand band) const noexcept;
    [[nodiscard]] std::uint32_t sampleCount(LevelBand band) const noexcept;

private:
    struct BandStats final {
        std::uint32_t count{0};
        float inputPowerMean{0.0F};
        float outputCurrentMean{0.0F};
    };
    std::array<BandStats, 5> bands_{};
};

}  // namespace dali::baseline
