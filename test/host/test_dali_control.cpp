#include <cstdlib>

#include "dali/control/dali_control.h"
#include "dali/protocol/dali_transaction.h"
#include "dali/sim/simulated_dali_phy.h"
#include "test_support.h"

namespace {
class NoDelay final : public dali::protocol::IDelayProvider { public: void delayMs(std::uint32_t) noexcept override {} };
class ZeroJitter final : public dali::protocol::IJitterSource { public: std::uint32_t next(std::uint32_t) noexcept override { return 0; } };
}

int main() {
    using namespace dali;
    using namespace dali::protocol;
    using namespace dali::control;
    SimulatedDaliPhy phy;
    CHECK_EQ(phy.init(), PhyResult::Ok);
    NoDelay delay;
    ZeroJitter jitter;
    DaliTransactionService transaction{phy, delay, jitter};
    DaliProtocolPipeline pipeline{transaction};
    DaliControl control{pipeline};
    const auto address = ShortAddress::fromRaw(3U);
    const auto group = GroupAddress::fromRaw(2U);
    CHECK_TRUE(address.has_value());
    CHECK_TRUE(group.has_value());
    CHECK_EQ(control.sendShort(*address, DaliCommand::GoToScene7), ControlResult::Ok);
    CHECK_EQ(phy.lastForwardFrame(), 0x0717U);
    CHECK_EQ(control.setShortLevel(*address, 170U), ControlResult::Ok);
    CHECK_EQ(phy.lastForwardFrame(), 0x06AAU);
    CHECK_EQ(control.sendGroup(*group, DaliCommand::AddToGroup2), ControlResult::Ok);
    CHECK_EQ(phy.lastForwardFrame(), 0x8562U);
    CHECK_EQ(control.setGroupLevel(*group, 255U), ControlResult::InvalidArgument);
    CHECK_EQ(control.setDtr0(0x2AU), ControlResult::Ok);
    CHECK_EQ(phy.lastForwardFrame(), 0xA32AU);
    return EXIT_SUCCESS;
}
