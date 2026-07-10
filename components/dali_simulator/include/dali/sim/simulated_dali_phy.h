#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "dali/phy/i_dali_phy.h"

namespace dali {

struct SimulatedResponse final {
    PhyResult result{PhyResult::Timeout};
    std::uint8_t value{0};
    std::uint32_t delayMs{0};
};

class SimulatedDaliPhy final : public IDaliPhy {
public:
    static constexpr std::size_t kResponseCapacity = 32;

    PhyResult init() noexcept override;
    PhyResult transmit(std::uint16_t forwardFrame) noexcept override;
    PhyResult receive(std::uint8_t& backwardFrame,
                      std::uint32_t timeoutMs) noexcept override;
    [[nodiscard]] bool isBusBusy() const noexcept override;
    [[nodiscard]] bool hasCollision() const noexcept override;
    [[nodiscard]] PhyHealth health() const noexcept override;
    PhyResult reset() noexcept override;

    PhyResult enqueueResponse(SimulatedResponse response) noexcept;
    void setBusPowered(bool powered) noexcept;
    void setTransceiverHealthy(bool healthy) noexcept;
    void setBusBusy(bool busy) noexcept;
    void setCollision(bool collision) noexcept;
    void setLineStuckHigh(bool stuck) noexcept;
    void setLineStuckLow(bool stuck) noexcept;
    [[nodiscard]] std::uint16_t lastForwardFrame() const noexcept;
    [[nodiscard]] std::uint32_t transmittedFrameCount() const noexcept;

private:
    std::array<SimulatedResponse, kResponseCapacity> responses_{};
    std::size_t responseHead_{0};
    std::size_t responseCount_{0};
    std::uint16_t lastForwardFrame_{0};
    std::uint32_t transmittedFrameCount_{0};
    PhyHealth health_{false, true, true, false, false};
    bool busBusy_{false};
    bool collision_{false};
};

}  // namespace dali
