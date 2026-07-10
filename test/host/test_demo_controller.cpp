#include <cstdlib>

#include "dali/demo/demo_controller.h"
#include "test_support.h"

int main() {
    dali::demo::DemoController demo;
    demo.step(0U);
    CHECK_EQ(demo.discoveredLights(), 4U);
    CHECK_TRUE(demo.c6MessagesSent() >= 4U);

    demo.step(2000U);
    CHECK_TRUE(demo.activeFaultEvents() >= 2U);
    const auto before = demo.c6MessagesSent();
    demo.step(4000U);
    CHECK_TRUE(demo.c6MessagesSent() >= before);
    CHECK_TRUE(demo.healthScore(1U) <= 20.0F);
    CHECK_TRUE(demo.healthScore(2U) < 100.0F);
    return EXIT_SUCCESS;
}
