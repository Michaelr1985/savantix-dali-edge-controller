#pragma once

#include <cstdint>

namespace dali {

enum class PhyResult : std::uint8_t {
    Ok,
    Timeout,
    Busy,
    Collision,
    BusPowerLost,
    TransceiverFault,
    InvalidArgument,
    NotInitialised,
    InternalError,
};

struct PhyHealth final {
    bool initialised{false};
    bool busPowered{false};
    bool transceiverHealthy{false};
    bool lineStuckHigh{false};
    bool lineStuckLow{false};
};

class IDaliPhy {
public:
    virtual ~IDaliPhy() = default;

    virtual PhyResult init() noexcept = 0;
    virtual PhyResult transmit(std::uint16_t forwardFrame) noexcept = 0;
    virtual PhyResult receive(std::uint8_t& backwardFrame,
                              std::uint32_t timeoutMs) noexcept = 0;
    [[nodiscard]] virtual bool isBusBusy() const noexcept = 0;
    [[nodiscard]] virtual bool hasCollision() const noexcept = 0;
    [[nodiscard]] virtual PhyHealth health() const noexcept = 0;
    virtual PhyResult reset() noexcept = 0;
};

}  // namespace dali
