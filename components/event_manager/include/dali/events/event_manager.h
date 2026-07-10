#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "dali/core/types.h"

namespace dali::events {

enum class EventAction : std::uint8_t { Activated, Escalated, Cleared, Reminder };

struct EventTransition final {
    LightEvent event{};
    EventAction action{EventAction::Activated};
    bool acknowledged{false};
};

class EventManager final {
public:
    static constexpr std::size_t kCapacity = 128;

    [[nodiscard]] std::optional<EventTransition> raise(
        LightEvent event) noexcept;
    [[nodiscard]] std::optional<EventTransition> clear(
        std::uint8_t address, LightEventType type,
        std::uint64_t timestamp) noexcept;
    [[nodiscard]] bool acknowledge(
        std::uint8_t address, LightEventType type) noexcept;
    [[nodiscard]] std::optional<EventTransition> reminder(
        std::uint64_t timestamp, std::uint64_t intervalMs) noexcept;

private:
    struct Active final {
        bool used{false};
        LightEvent event{};
        bool acknowledged{false};
        std::uint64_t lastEmitted{0};
    };

    [[nodiscard]] Active* find(std::uint8_t address,
                               LightEventType type) noexcept;
    std::array<Active, kCapacity> active_{};
    std::uint32_t nextEventId_{1};
};

}  // namespace dali::events
