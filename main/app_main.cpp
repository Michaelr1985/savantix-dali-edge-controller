#include "esp_log.h"
#include "app_config.h"

extern "C" void app_main() {
    const SmartLightConfig config = SmartLightConfig::fromKconfig();
    if (!config.validate()) {
        ESP_LOGE("savantix", "Invalid Smart-Light configuration");
        return;
    }
    ESP_LOGI("savantix", "DALI edge controller foundation booted for %u lights",
             static_cast<unsigned>(config.maximumLights));
}
