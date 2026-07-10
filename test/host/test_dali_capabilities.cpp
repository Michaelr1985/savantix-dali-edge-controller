#include <array>
#include <cstdlib>
#include <optional>

#include "dali/device/dali_device_manager.h"
#include "dali/phy/i_dali_phy.h"
#include "dali/protocol/i_dali_query_client.h"
#include "test_support.h"

namespace {

class CapabilityClient final : public dali::protocol::IDaliQueryClient {
public:
    dali::protocol::DaliQueryResult query(
        dali::ShortAddress address,
        dali::protocol::DaliCommand command,
        dali::protocol::CommandPriority) noexcept override {
        ++calls_[address.value()][static_cast<std::size_t>(command)];
        const auto value = values_[address.value()][static_cast<std::size_t>(command)];
        if (value.has_value()) {
            return {dali::protocol::DaliQueryOutcome::Ok,
                    dali::Reading<std::uint8_t>::valid(*value, 0, 0), 1,
                    dali::PhyResult::Ok};
        }
        return {dali::protocol::DaliQueryOutcome::NoResponse,
                dali::Reading<std::uint8_t>::temporarilyUnavailable(), 1,
                dali::PhyResult::Timeout};
    }

    void set(std::uint8_t address, dali::protocol::DaliCommand command,
             std::uint8_t value) noexcept {
        values_[address][static_cast<std::size_t>(command)] = value;
    }

    [[nodiscard]] std::uint32_t calls(
        std::uint8_t address, dali::protocol::DaliCommand command) const noexcept {
        return calls_[address][static_cast<std::size_t>(command)];
    }

private:
    static constexpr std::size_t kCommandCount = 0xC5;
    std::array<std::array<std::optional<std::uint8_t>, kCommandCount>, 64> values_{};
    std::array<std::array<std::uint32_t, kCommandCount>, 64> calls_{};
};

}  // namespace

int main() {
    using namespace dali;
    using namespace dali::device;
    using namespace dali::protocol;

    CapabilityClient client;
    const ShortAddress supported = *ShortAddress::fromRaw(4U);
    client.set(4U, DaliCommand::QueryStatus, 0U);
    client.set(4U, DaliCommand::QueryDeviceType, 8U);
    client.set(4U, DaliCommand::QueryVersionNumber, 2U);
    client.set(4U, DaliCommand::QueryPhysicalMinimum, 1U);

    DaliDeviceManager manager{client};
    (void)manager.registry().upsert(supported, 10U);
    (void)manager.detectCoreCapabilities(supported, 20U);
    const DaliDeviceRecord* record = manager.registry().find(supported);
    CHECK_TRUE(record != nullptr);
    CHECK_EQ(record->capabilities.at(CapabilityFeature::BasicStatus).state,
             CapabilityState::Supported);
    CHECK_EQ(record->capabilities.at(CapabilityFeature::DeviceType).state,
             CapabilityState::Supported);
    CHECK_EQ(record->deviceType.value, 8U);
    CHECK_EQ(record->physicalMinimum.value, 1U);

    const ShortAddress unsupported = *ShortAddress::fromRaw(5U);
    (void)manager.registry().upsert(unsupported, 10U);
    (void)manager.detectCoreCapabilities(unsupported, 20U);
    (void)manager.detectCoreCapabilities(unsupported, 30U);
    CHECK_EQ(manager.registry().find(unsupported)->capabilities
                 .at(CapabilityFeature::DeviceType).state,
             CapabilityState::Unsupported);
    const auto before = client.calls(5U, DaliCommand::QueryDeviceType);
    (void)manager.detectCoreCapabilities(unsupported, 40U);
    CHECK_EQ(client.calls(5U, DaliCommand::QueryDeviceType), before);
    return EXIT_SUCCESS;
}
