#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"
#include "foundation_runtime.h"
#include "dali/core/types.h"
#include "dali/protocol/dali_commands.h"
#include "dali/protocol/dali_frame.h"
#include "dali/protocol/dali_protocol_pipeline.h"
#include "dali/protocol/dali_response_parser.h"
#include "dali/protocol/dali_transaction.h"
#include "dali/sim/simulated_dali_phy.h"
#include "dali/demo/demo_controller.h"
#include "esp_timer.h"

namespace {

class FreeRtosDelay final : public dali::protocol::IDelayProvider {
public:
    void delayMs(std::uint32_t delayMs) noexcept override {
        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
};

class LcgJitter final : public dali::protocol::IJitterSource {
public:
    std::uint32_t next(std::uint32_t maxInclusive) noexcept override {
        state_ = state_ * 1664525U + 1013904223U;
        return state_ % (maxInclusive + 1U);
    }

private:
    std::uint32_t state_{0x534C5301U};
};

}  // namespace

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

    const auto address = dali::ShortAddress::fromRaw(3U);
    if (!address.has_value() ||
        simulatedPhy.enqueueResponse({dali::PhyResult::Ok, 0x04U, 0U}) !=
            dali::PhyResult::Ok) {
        ESP_LOGE("savantix", "Protocol self-check setup failed");
        return;
    }

    FreeRtosDelay delay;
    LcgJitter jitter;
    dali::protocol::DaliTransactionService transaction{
        simulatedPhy, delay, jitter};
    dali::protocol::DaliProtocolPipeline pipeline{transaction};
    const dali::protocol::DaliRequest request{
        dali::protocol::makeShortCommand(
            *address, dali::protocol::DaliCommand::QueryStatus),
        true,
        20U,
        config.maxCommandRetries,
        dali::protocol::CommandPriority::Normal,
        address->value(),
        1U,
    };
    if (pipeline.enqueue(request) != dali::protocol::QueueResult::Ok) {
        ESP_LOGE("savantix", "Protocol self-check queue full");
        return;
    }
    const auto processed = pipeline.processNext();
    if (!processed.has_value() ||
        processed->result.status != dali::protocol::TransactionStatus::Ok ||
        !processed->result.response.has_value()) {
        ESP_LOGE("savantix", "Protocol self-check transaction failed");
        return;
    }
    const auto status = dali::protocol::parseBasicStatus(
        dali::protocol::makeBackwardResponse(
            processed->result.lastPhyResult,
            *processed->result.response));
    if (!status.hasValue() || !status.value.lampPowerOn) {
        ESP_LOGE("savantix", "Protocol self-check response invalid");
        return;
    }
    ESP_LOGI("savantix",
             "Protocol self-check passed for simulated address %u",
             static_cast<unsigned>(address->value()));

    static dali::demo::DemoController demo;
    xTaskCreate(
        [](void* context) {
            auto* controller = static_cast<dali::demo::DemoController*>(context);
            while (true) {
                controller->step(static_cast<std::uint64_t>(esp_timer_get_time() / 1000));
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        },
        "dali_demo", 6144, &demo, 4, nullptr);
}
