#include <cstddef>
#include <cstdlib>

#include "dali/sim/simulated_dali_phy.h"
#include "test_support.h"

int main() {
    using namespace dali;
    SimulatedDaliPhy phy;
    CHECK_EQ(phy.init(), PhyResult::Ok);

    for (std::size_t index = 0; index < SimulatedDaliPhy::kResponseCapacity;
         ++index) {
        CHECK_EQ(phy.enqueueResponse({PhyResult::Ok,
                                     static_cast<std::uint8_t>(index), 0U}),
                 PhyResult::Ok);
    }
    CHECK_EQ(phy.enqueueResponse({PhyResult::Ok, 0U, 0U}), PhyResult::Busy);

    std::uint8_t response = 0;
    CHECK_EQ(phy.receive(response, 0U), PhyResult::Ok);
    CHECK_EQ(response, 0U);
    CHECK_EQ(phy.enqueueResponse({PhyResult::Ok, 0xAAU, 0U}), PhyResult::Ok);
    return EXIT_SUCCESS;
}
