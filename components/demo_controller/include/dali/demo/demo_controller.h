#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "dali/c6/c6_interface.h"
#include "dali/device/dali_device_manager.h"
#include "dali/events/event_manager.h"
#include "dali/health/light_health_engine.h"
#include "dali/phy/i_dali_phy.h"
#include "dali/protocol/i_dali_query_client.h"

namespace dali::demo {

class DemoController final {
public:
    DemoController() noexcept;
    void step(std::uint64_t nowMs) noexcept;
    [[nodiscard]] std::size_t discoveredLights() const noexcept { return discoveredLights_; }
    [[nodiscard]] std::size_t c6MessagesSent() const noexcept { return transport_.sentCount(); }
    [[nodiscard]] std::size_t activeFaultEvents() const noexcept { return events_.activeCount(); }
    [[nodiscard]] float healthScore(std::uint8_t address) const noexcept {
        return address < health_.size() ? health_[address].score() : 0.0F;
    }

private:
    class DemoQueryClient final : public protocol::IDaliQueryClient {
    public:
        void setNow(std::uint64_t nowMs) noexcept { nowMs_ = nowMs; }
        protocol::DaliQueryResult query(
            ShortAddress address, protocol::DaliCommand command,
            protocol::CommandPriority) noexcept override;
    private:
        std::uint64_t nowMs_{0};
    };

    void publish(const LightEvent& event) noexcept;
    DemoQueryClient client_{};
    device::DaliDeviceManager devices_;
    events::EventManager events_{};
    c6::MockC6Transport transport_{};
    c6::C6Session c6_;
    std::array<health::LightHealthEngine, 4> health_{};
    std::size_t discoveredLights_{0};
};

}  // namespace dali::demo
