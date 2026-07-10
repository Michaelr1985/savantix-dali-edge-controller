#include <cstdlib>

#include "dali/events/event_manager.h"
#include "test_support.h"

int main() {
    using namespace dali;
    using namespace dali::events;

    EventManager manager;
    LightEvent event{};
    event.shortAddress = 3U;
    event.type = LightEventType::ThermalWarning;
    event.severity = EventSeverity::Warning;
    event.timestamp = 100U;

    const auto activated = manager.raise(event);
    CHECK_TRUE(activated.has_value());
    CHECK_EQ(activated->action, EventAction::Activated);
    CHECK_TRUE(!manager.raise(event).has_value());

    event.severity = EventSeverity::Critical;
    event.timestamp = 200U;
    const auto escalated = manager.raise(event);
    CHECK_TRUE(escalated.has_value());
    CHECK_EQ(escalated->action, EventAction::Escalated);

    CHECK_TRUE(manager.acknowledge(3U, LightEventType::ThermalWarning));
    const auto cleared = manager.clear(3U, LightEventType::ThermalWarning, 300U);
    CHECK_TRUE(cleared.has_value());
    CHECK_EQ(cleared->action, EventAction::Cleared);
    CHECK_TRUE(!manager.clear(3U, LightEventType::ThermalWarning, 400U).has_value());
    return EXIT_SUCCESS;
}
