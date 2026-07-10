#include <cstdlib>

#include "dali/core/types.h"
#include "dali/device/dali_device_manager.h"
#include "test_support.h"

int main() {
    using namespace dali;
    using namespace dali::device;

    DaliDeviceRegistry registry;
    const ShortAddress address0 = *ShortAddress::fromRaw(0U);
    const ShortAddress address63 = *ShortAddress::fromRaw(63U);
    CHECK_TRUE(registry.find(address0) == nullptr);
    CHECK_EQ(registry.count(), 0U);

    DaliDeviceRecord& first = registry.upsert(address0, 100U);
    CHECK_EQ(first.address.value(), 0U);
    CHECK_EQ(first.firstSeenUptimeMs, 100U);
    CHECK_EQ(registry.count(), 1U);

    DaliDeviceRecord& last = registry.upsert(address63, 200U);
    CHECK_EQ(last.address.value(), 63U);
    CHECK_EQ(registry.count(), 2U);

    DaliDeviceRecord& same = registry.upsert(address0, 999U);
    CHECK_TRUE(&same == &first);
    CHECK_EQ(same.firstSeenUptimeMs, 100U);
    CHECK_EQ(registry.count(), 2U);
    CHECK_TRUE(!ShortAddress::fromRaw(64U).has_value());
    return EXIT_SUCCESS;
}
