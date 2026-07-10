#include "app_config.h"

static_assert(SmartLightConfig::kMaximumDaliLights == 64);

int main() {
    const SmartLightConfig config = SmartLightConfig::fromKconfig();
    return config.validate() ? 0 : 1;
}
