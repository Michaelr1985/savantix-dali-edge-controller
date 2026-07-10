#include <cstdlib>

#include "dali/storage/local_storage.h"
#include "test_support.h"

int main() {
    using namespace dali;
    using namespace dali::storage;
    EventHistory history;
    LightEvent firstEvent{};
    firstEvent.eventId = 1U;
    firstEvent.shortAddress = 3U;
    firstEvent.type = LightEventType::ThermalWarning;
    firstEvent.severity = EventSeverity::Warning;
    LightEvent secondEvent = firstEvent;
    secondEvent.eventId = 2U;
    StoredEvent first{1U, 100U, firstEvent};
    StoredEvent second{2U, 200U, secondEvent};
    CHECK_TRUE(history.append(first));
    CHECK_TRUE(history.append(second));
    CHECK_EQ(history.size(), 2U);
    const auto records = history.recover();
    CHECK_EQ(records.size, 2U);
    CHECK_EQ(records.records[0].sequence, 1U);
    CHECK_EQ(records.records[1].sequence, 2U);

    history.corruptNewestForTest();
    const auto recovered = history.recover();
    CHECK_EQ(recovered.size, 1U);
    CHECK_EQ(recovered.records[0].sequence, 1U);
    return EXIT_SUCCESS;
}
