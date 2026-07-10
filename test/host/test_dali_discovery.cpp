#include <array>
#include <cstdlib>

#include "dali/device/dali_device_manager.h"
#include "dali/phy/i_dali_phy.h"
#include "dali/protocol/i_dali_query_client.h"
#include "test_support.h"

namespace {

class ScriptedClient final : public dali::protocol::IDaliQueryClient {
public:
    struct Slot final {
        dali::protocol::DaliQueryResult result{};
        std::uint32_t calls{0};
    };

    dali::protocol::DaliQueryResult query(
        dali::ShortAddress address,
        dali::protocol::DaliCommand command,
        dali::protocol::CommandPriority) noexcept override {
        ++slots_[address.value()].calls;
        if (command == dali::protocol::DaliCommand::QueryControlGearPresent) {
            return slots_[address.value()].result;
        }
        return {dali::protocol::DaliQueryOutcome::NoResponse,
                dali::Reading<std::uint8_t>::temporarilyUnavailable(), 1,
                dali::PhyResult::Timeout};
    }

    void set(std::uint8_t address, dali::protocol::DaliQueryOutcome outcome,
             std::uint8_t value = 0U) noexcept {
        slots_[address].result = {
            outcome,
            outcome == dali::protocol::DaliQueryOutcome::Ok
                ? dali::Reading<std::uint8_t>::valid(value, 0, 0)
                : dali::Reading<std::uint8_t>::temporarilyUnavailable(),
            1,
            outcome == dali::protocol::DaliQueryOutcome::Ok
                ? dali::PhyResult::Ok
                : outcome == dali::protocol::DaliQueryOutcome::Collision
                    ? dali::PhyResult::Collision
                    : dali::PhyResult::Timeout};
    }

    [[nodiscard]] std::uint32_t calls(std::uint8_t address) const noexcept {
        return slots_[address].calls;
    }

private:
    std::array<Slot, 64> slots_{};
};

}  // namespace

int main() {
    using namespace dali;
    using namespace dali::device;
    using namespace dali::protocol;

    ScriptedClient client;
    client.set(0U, DaliQueryOutcome::Ok, 0U);
    client.set(3U, DaliQueryOutcome::Ok, 0U);
    client.set(7U, DaliQueryOutcome::Collision);
    DaliDeviceManager manager{client};

    const DiscoveryBatch first = manager.scanPresence(100U);
    CHECK_EQ(first.size, 3U);
    CHECK_EQ(first.changes[0].type, DiscoveryChangeType::DeviceDiscovered);
    CHECK_EQ(first.changes[1].type, DiscoveryChangeType::DeviceDiscovered);
    CHECK_EQ(first.changes[2].type,
             DiscoveryChangeType::DuplicateAddressSuspected);
    CHECK_EQ(manager.registry().count(), 2U);

    const DiscoveryBatch repeat = manager.scanPresence(200U);
    CHECK_EQ(repeat.size, 0U);

    client.set(3U, DaliQueryOutcome::NoResponse);
    for (std::uint64_t now : {300U, 400U}) {
        CHECK_EQ(manager.scanPresence(now).size, 0U);
    }
    const DiscoveryBatch missing = manager.scanPresence(500U);
    CHECK_EQ(missing.size, 1U);
    CHECK_EQ(missing.changes[0].type, DiscoveryChangeType::DeviceMissing);

    const DiscoveryBatch stillMissing = manager.scanPresence(600U);
    CHECK_EQ(stillMissing.size, 0U);
    client.set(3U, DaliQueryOutcome::Ok, 0U);
    const DiscoveryBatch recovered = manager.scanPresence(700U);
    CHECK_EQ(recovered.size, 1U);
    CHECK_EQ(recovered.changes[0].type, DiscoveryChangeType::DeviceRecovered);

    client.set(3U, DaliQueryOutcome::BusFailure);
    CHECK_EQ(manager.scanPresence(800U).size, 0U);
    CHECK_TRUE(manager.registry().find(*ShortAddress::fromRaw(3U))->present);
    return EXIT_SUCCESS;
}
