#include "dali/protocol/dali_response_parser.h"

namespace dali::protocol {
namespace {

ReadingQuality qualityFromPhyResult(PhyResult result) noexcept {
    switch (result) {
        case PhyResult::Ok:
            return ReadingQuality::Valid;
        case PhyResult::Timeout:
        case PhyResult::Busy:
        case PhyResult::Collision:
            return ReadingQuality::TemporarilyUnavailable;
        case PhyResult::BusPowerLost:
        case PhyResult::TransceiverFault:
        case PhyResult::InvalidArgument:
        case PhyResult::NotInitialised:
        case PhyResult::InternalError:
            return ReadingQuality::Invalid;
    }
    return ReadingQuality::Invalid;
}

template <typename T>
Reading<T> unavailableReading(ReadingQuality quality) noexcept {
    return {{}, quality, 0, 0, false};
}

}  // namespace

BackwardResponse makeBackwardResponse(PhyResult result,
                                      std::uint8_t value) noexcept {
    return {value, qualityFromPhyResult(result)};
}

Reading<bool> parseYesNo(const BackwardResponse& response) noexcept {
    if (response.quality != ReadingQuality::Valid) {
        return unavailableReading<bool>(response.quality);
    }
    return Reading<bool>::valid(response.value != 0U, 0, 0);
}

Reading<BasicStatus> parseBasicStatus(
    const BackwardResponse& response) noexcept {
    if (response.quality != ReadingQuality::Valid) {
        return unavailableReading<BasicStatus>(response.quality);
    }
    const std::uint8_t value = response.value;
    const BasicStatus status{
        (value & 0x01U) != 0U,
        (value & 0x02U) != 0U,
        (value & 0x04U) != 0U,
        (value & 0x08U) != 0U,
        (value & 0x10U) != 0U,
        (value & 0x20U) != 0U,
        (value & 0x40U) != 0U,
        (value & 0x80U) != 0U,
    };
    return Reading<BasicStatus>::valid(status, 0, 0);
}

}  // namespace dali::protocol
