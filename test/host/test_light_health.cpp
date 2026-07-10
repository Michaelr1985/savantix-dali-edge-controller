#include <cstdlib>

#include "dali/health/light_health_engine.h"
#include "dali/trend/trend_engine.h"
#include "test_support.h"

int main() {
    using namespace dali::health;

    FaultPersistence persistence{3U, 5U, 10U};
    CHECK_TRUE(!persistence.observe(true));
    CHECK_TRUE(!persistence.warningActive());
    persistence.observe(true);
    persistence.observe(true);
    CHECK_TRUE(persistence.warningActive());
    CHECK_TRUE(!persistence.faultActive());
    persistence.observe(true);
    persistence.observe(true);
    CHECK_TRUE(persistence.faultActive());
    for (int i = 0; i < 10; ++i) persistence.observe(false);
    CHECK_TRUE(!persistence.warningActive());
    CHECK_TRUE(!persistence.faultActive());

    LightHealthEngine engine;
    HealthObservation failed{};
    failed.commandedOn = true;
    failed.lampFailure = true;
    auto result = engine.evaluate(failed);
    CHECK_EQ(result.state, dali::LightState::Failed);
    CHECK_TRUE(result.healthScore <= 20.0F);

    HealthObservation healthy{};
    healthy.commandedOn = true;
    for (int i = 0; i < 5; ++i) {
        result = engine.evaluate(healthy);
    }
    CHECK_TRUE(result.healthScore < 100.0F);
    return EXIT_SUCCESS;
}
