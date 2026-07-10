#include <cstdlib>

#include "dali/sim/simulated_dali_phy.h"
#include "test_support.h"

int main() {
    using namespace dali;
    SimulatedDaliPhy phy;
    std::uint8_t response = 0;

    CHECK_EQ(phy.transmit(0x0190U), PhyResult::NotInitialised);
    CHECK_EQ(phy.init(), PhyResult::Ok);
    CHECK_TRUE(phy.health().busPowered);

    CHECK_EQ(phy.enqueueResponse({PhyResult::Ok, 0xFFU, 3U}), PhyResult::Ok);
    CHECK_EQ(phy.transmit(0x0190U), PhyResult::Ok);
    CHECK_EQ(phy.lastForwardFrame(), 0x0190U);
    CHECK_EQ(phy.transmittedFrameCount(), 1U);
    CHECK_EQ(phy.receive(response, 2U), PhyResult::Timeout);
    CHECK_EQ(phy.receive(response, 3U), PhyResult::Ok);
    CHECK_EQ(response, 0xFFU);

    phy.setBusPowered(false);
    CHECK_EQ(phy.transmit(0x0190U), PhyResult::BusPowerLost);
    CHECK_EQ(phy.reset(), PhyResult::Ok);
    CHECK_TRUE(!phy.health().busPowered);
    return EXIT_SUCCESS;
}
