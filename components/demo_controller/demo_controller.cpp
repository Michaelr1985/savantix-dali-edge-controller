#include "dali/demo/demo_controller.h"

namespace dali::demo {

DemoController::DemoController() noexcept
    : devices_{client_}, c6_{transport_, 3U} {}

protocol::DaliQueryResult DemoController::DemoQueryClient::query(
    ShortAddress address, protocol::DaliCommand command,
    protocol::CommandPriority) noexcept {
    if (address.value() > 3U ||
        (address.value() == 3U &&
         command == protocol::DaliCommand::QueryControlGearPresent &&
         ((nowMs_ / 2000U) % 2U) == 1U)) {
        return {protocol::DaliQueryOutcome::NoResponse,
                Reading<std::uint8_t>::temporarilyUnavailable(), 1,
                PhyResult::Timeout};
    }
    return {protocol::DaliQueryOutcome::Ok,
            Reading<std::uint8_t>::valid(0U, nowMs_, 0), 1,
            PhyResult::Ok};
}

void DemoController::publish(const LightEvent& event) noexcept {
    c6::Frame frame{};
    frame.type = c6::MessageType::LightEvent;
    frame.payload[0] = event.shortAddress;
    frame.payload[1] = static_cast<std::uint8_t>(event.type);
    frame.payload[2] = static_cast<std::uint8_t>(event.severity);
    frame.payloadLength = 3U;
    (void)c6_.publish(frame);
}

void DemoController::step(std::uint64_t nowMs) noexcept {
    client_.setNow(nowMs);
    const device::DiscoveryBatch changes = devices_.scanPresence(nowMs);
    for (std::size_t i = 0; i < changes.size; ++i) {
        const auto& change = changes.changes[i];
        LightEvent event{};
        event.shortAddress = change.address.value();
        event.timestamp = nowMs;
        event.type = change.type == device::DiscoveryChangeType::DeviceMissing
                         ? LightEventType::DeviceMissing
                         : change.type == device::DiscoveryChangeType::DeviceRecovered
                             ? LightEventType::DeviceRecovered
                             : LightEventType::DeviceDiscovered;
        event.severity = event.type == LightEventType::DeviceMissing
                             ? EventSeverity::Critical : EventSeverity::Info;
        if (events_.raise(event).has_value()) publish(event);
        if (change.type == device::DiscoveryChangeType::DeviceDiscovered) ++discoveredLights_;
    }

    for (std::uint8_t address = 0; address < 4U; ++address) {
        health::HealthObservation observation{};
        observation.commandedOn = true;
        observation.lampFailure = address == 1U;
        observation.thermalWarning = address == 2U && nowMs >= 1000U;
        observation.intermittentFault = address == 3U && ((nowMs / 2000U) % 2U) == 1U;
        const health::HealthEvaluation evaluation = health_[address].evaluate(observation);
        if (observation.lampFailure || observation.thermalWarning ||
            observation.intermittentFault) {
            LightEvent event{};
            event.shortAddress = address;
            event.timestamp = nowMs;
            event.healthScore = evaluation.healthScore;
            event.activeFaultFlags = evaluation.activeFaultFlags;
            event.type = observation.lampFailure ? LightEventType::LampFailure
                       : observation.thermalWarning ? LightEventType::ThermalWarning
                       : LightEventType::IntermittentFault;
            event.severity = observation.lampFailure ? EventSeverity::Critical
                                                     : EventSeverity::Warning;
            if (events_.raise(event).has_value()) publish(event);
        }
    }
}

}  // namespace dali::demo
