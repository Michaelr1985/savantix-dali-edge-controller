#include <array>
#include <cstdlib>
#include <optional>

#include "dali/device/dali_device_manager.h"
#include "dali/phy/i_dali_phy.h"
#include "dali/protocol/i_dali_query_client.h"
#include "test_support.h"

namespace {

class MultiDeviceClient final : public dali::protocol::IDaliQueryClient {
public:
    dali::protocol::DaliQueryResult query(
        dali::ShortAddress address, dali::protocol::DaliCommand command,
        dali::protocol::CommandPriority) noexcept override {
        if (address.value() == 7U) {
            return {dali::protocol::DaliQueryOutcome::Collision,
                    dali::Reading<std::uint8_t>::temporarilyUnavailable(), 1,
                    dali::PhyResult::Collision};
        }
        if (address.value() > 3U) {
            return {dali::protocol::DaliQueryOutcome::NoResponse,
                    dali::Reading<std::uint8_t>::temporarilyUnavailable(), 1,
                    dali::PhyResult::Timeout};
        }
        const std::uint8_t value =
            command == dali::protocol::DaliCommand::QueryDeviceType
                ? static_cast<std::uint8_t>(address.value() + 8U)
                : command == dali::protocol::DaliCommand::QueryVersionNumber
                    ? 2U
                    : command == dali::protocol::DaliCommand::QueryPhysicalMinimum
                        ? 1U
                        : 0U;
        return {dali::protocol::DaliQueryOutcome::Ok,
                dali::Reading<std::uint8_t>::valid(value, 0, 0), 1,
                dali::PhyResult::Ok};
    }
};

}  // namespace

int main() {
    using namespace dali;
    using namespace dali::device;

    MultiDeviceClient client;
    DaliDeviceManager manager{client};
    const DiscoveryBatch first = manager.scanPresence(100U);
    CHECK_EQ(first.size, 5U);
    CHECK_EQ(manager.registry().count(), 4U);
    CHECK_EQ(first.changes[4].type,
             DiscoveryChangeType::DuplicateAddressSuspected);

    for (std::uint8_t address = 0; address < 3U; ++address) {
        const auto shortAddress = *ShortAddress::fromRaw(address);
        (void)manager.detectCoreCapabilities(shortAddress, 200U);
        const DaliDeviceRecord* record = manager.registry().find(shortAddress);
        CHECK_TRUE(record != nullptr);
        CHECK_EQ(record->capabilities.at(CapabilityFeature::BasicStatus).state,
                 CapabilityState::Supported);
        CHECK_EQ(record->capabilities.at(CapabilityFeature::DeviceType).state,
                 CapabilityState::Supported);
    }

    CHECK_EQ(manager.scanPresence(300U).size, 0U);
    return EXIT_SUCCESS;
}
