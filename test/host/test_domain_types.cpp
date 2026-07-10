#include <cstdlib>

#include "dali/core/types.h"
#include "test_support.h"

int main() {
    using namespace dali;
    const auto address = ShortAddress::fromRaw(63);
    CHECK_TRUE(address.has_value());
    CHECK_EQ(address->value(), 63U);
    CHECK_TRUE(!ShortAddress::fromRaw(64).has_value());

    const auto group = GroupAddress::fromRaw(15);
    CHECK_TRUE(group.has_value());
    CHECK_TRUE(!GroupAddress::fromRaw(16).has_value());

    const Reading<float> unsupported = Reading<float>::unsupported();
    CHECK_TRUE(!unsupported.hasValue());
    CHECK_EQ(unsupported.quality, ReadingQuality::Unsupported);

    const Reading<float> measured = Reading<float>::valid(230.5F, 1000U, 0U);
    CHECK_TRUE(measured.hasValue());
    CHECK_EQ(measured.value, 230.5F);
    CHECK_EQ(measured.uptimeMs, 1000U);
    CHECK_TRUE(!measured.unixTimeValid);
    return EXIT_SUCCESS;
}
