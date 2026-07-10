#include <cstdlib>

#include "app_config.h"
#include "foundation_runtime.h"
#include "dali/sim/simulated_dali_phy.h"
#include "test_support.h"

int main() {
    using namespace dali;

    SmartLightConfig invalid = SmartLightConfig::defaults();
    invalid.maximumLights = 0;
    SimulatedDaliPhy invalidPhy;
    CHECK_EQ(initialiseFoundation(invalid, invalidPhy),
             FoundationResult::InvalidConfiguration);
    CHECK_TRUE(!invalidPhy.health().initialised);

    const SmartLightConfig config = SmartLightConfig::defaults();
    SimulatedDaliPhy phy;
    CHECK_EQ(initialiseFoundation(config, phy), FoundationResult::Ready);
    CHECK_TRUE(phy.health().initialised);
    CHECK_TRUE(phy.health().busPowered);
    CHECK_TRUE(phy.health().transceiverHealthy);
    return EXIT_SUCCESS;
}
