#include <cstdlib>

#include "dali/core/types.h"
#include "dali/protocol/dali_commands.h"
#include "dali/protocol/dali_frame.h"
#include "test_support.h"

int main() {
    using namespace dali;
    using namespace dali::protocol;

    const auto shortAddress = ShortAddress::fromRaw(3);
    const auto groupAddress = GroupAddress::fromRaw(2);
    const auto level127 = ArcLevel::fromRaw(127);
    const auto level254 = ArcLevel::fromRaw(254);
    const auto level42 = ArcLevel::fromRaw(42);
    CHECK_TRUE(shortAddress.has_value());
    CHECK_TRUE(groupAddress.has_value());
    CHECK_TRUE(level127.has_value());
    CHECK_TRUE(level254.has_value());
    CHECK_TRUE(level42.has_value());

    CHECK_EQ(makeShortCommand(*shortAddress, DaliCommand::QueryStatus).raw,
             0x0790U);
    CHECK_EQ(makeShortArc(*shortAddress, *level127).raw, 0x067FU);
    CHECK_EQ(makeGroupCommand(*groupAddress, DaliCommand::Off).raw, 0x8500U);
    CHECK_EQ(makeGroupArc(*groupAddress, *level254).raw, 0x84FEU);
    CHECK_EQ(makeBroadcastCommand(DaliCommand::QueryStatus).raw, 0xFF90U);
    CHECK_EQ(makeBroadcastArc(*level42).raw, 0xFE2AU);
    CHECK_TRUE(!ArcLevel::fromRaw(255U).has_value());

    const ForwardFrame frame{0xA55AU};
    CHECK_EQ(frame.addressByte(), 0xA5U);
    CHECK_EQ(frame.dataByte(), 0x5AU);
    return EXIT_SUCCESS;
}
