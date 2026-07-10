#include "dali/baseline/baseline_engine.h"

namespace dali::baseline {

LevelBand BaselineStore::bandFor(std::uint8_t level) noexcept {
    if (level <= 20U) return LevelBand::Band1;
    if (level <= 40U) return LevelBand::Band2;
    if (level <= 60U) return LevelBand::Band3;
    if (level <= 80U) return LevelBand::Band4;
    return LevelBand::Band5;
}

bool BaselineStore::update(std::uint8_t commandedLevel,
                           const BaselineObservation& observation) noexcept {
    if (!observation.stable || !observation.healthy ||
        !observation.communicationAcceptable ||
        !observation.inputPower.hasValue() ||
        !observation.outputCurrent.hasValue()) return false;
    BandStats& stats = bands_[static_cast<std::size_t>(bandFor(commandedLevel))];
    ++stats.count;
    const float count = static_cast<float>(stats.count);
    stats.inputPowerMean += (observation.inputPower.value - stats.inputPowerMean) / count;
    stats.outputCurrentMean +=
        (observation.outputCurrent.value - stats.outputCurrentMean) / count;
    return true;
}

float BaselineStore::meanInputPower(LevelBand band) const noexcept {
    return bands_[static_cast<std::size_t>(band)].inputPowerMean;
}

float BaselineStore::meanOutputCurrent(LevelBand band) const noexcept {
    return bands_[static_cast<std::size_t>(band)].outputCurrentMean;
}

std::uint32_t BaselineStore::sampleCount(LevelBand band) const noexcept {
    return bands_[static_cast<std::size_t>(band)].count;
}

}  // namespace dali::baseline
