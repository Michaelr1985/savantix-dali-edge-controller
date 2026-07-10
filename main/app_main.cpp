#include "esp_log.h"

#include "app_config.h"
#include "foundation_runtime.h"
#include "dali/sim/simulated_dali_phy.h"

extern "C" void app_main() {
    const SmartLightConfig config = SmartLightConfig::fromKconfig();
    if (!config.validate()) {
        ESP_LOGE("savantix", "Invalid Smart-Light configuration");
        return;
    }

    if (!config.demoEnabled) {
        ESP_LOGW("savantix", "No hardware DALI PHY is selected in Phase 1");
        return;
    }

    static dali::SimulatedDaliPhy simulatedPhy;
    const FoundationResult result = initialiseFoundation(config, simulatedPhy);
    if (result != FoundationResult::Ready) {
        ESP_LOGE("savantix", "Foundation initialisation failed: %u",
                 static_cast<unsigned>(result));
        return;
    }

    ESP_LOGI("savantix",
             "DALI foundation ready for up to %u simulated lights",
             static_cast<unsigned>(config.maximumLights));
}
