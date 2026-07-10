#include "dali/events/event_manager.h"

namespace dali::events {

EventManager::Active* EventManager::find(
    std::uint8_t address, LightEventType type) noexcept {
    for (Active& item : active_) {
        if (item.used && item.event.shortAddress == address && item.event.type == type) {
            return &item;
        }
    }
    return nullptr;
}

std::optional<EventTransition> EventManager::raise(LightEvent event) noexcept {
    Active* existing = find(event.shortAddress, event.type);
    if (existing == nullptr) {
        for (Active& item : active_) {
            if (!item.used) {
                if (event.eventId == 0U) event.eventId = nextEventId_++;
                item = {true, event, false, event.timestamp};
                return EventTransition{event, EventAction::Activated, false};
            }
        }
        return std::nullopt;
    }
    if (static_cast<std::uint8_t>(event.severity) >
        static_cast<std::uint8_t>(existing->event.severity)) {
        existing->event = event;
        existing->lastEmitted = event.timestamp;
        return EventTransition{event, EventAction::Escalated, existing->acknowledged};
    }
    existing->event = event;
    return std::nullopt;
}

std::optional<EventTransition> EventManager::clear(
    std::uint8_t address, LightEventType type,
    std::uint64_t timestamp) noexcept {
    Active* existing = find(address, type);
    if (existing == nullptr) return std::nullopt;
    existing->event.timestamp = timestamp;
    EventTransition transition{existing->event, EventAction::Cleared,
                               existing->acknowledged};
    existing->used = false;
    return transition;
}

bool EventManager::acknowledge(std::uint8_t address,
                               LightEventType type) noexcept {
    Active* existing = find(address, type);
    if (existing == nullptr) return false;
    existing->acknowledged = true;
    return true;
}

std::optional<EventTransition> EventManager::reminder(
    std::uint64_t timestamp, std::uint64_t intervalMs) noexcept {
    for (Active& item : active_) {
        if (!item.used || timestamp < item.lastEmitted ||
            timestamp - item.lastEmitted < intervalMs) continue;
        item.lastEmitted = timestamp;
        item.event.timestamp = timestamp;
        return EventTransition{item.event, EventAction::Reminder,
                               item.acknowledged};
    }
    return std::nullopt;
}

}  // namespace dali::events
