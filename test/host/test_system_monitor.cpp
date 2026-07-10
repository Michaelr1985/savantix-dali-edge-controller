#include <cstdlib>

#include "dali/monitor/system_monitor.h"
#include "test_support.h"

int main() {
    dali::monitor::SystemMonitor monitor;
    monitor.setExpectedTaskCount(2U);
    monitor.heartbeat(0U, 100U);
    monitor.heartbeat(1U, 100U);
    CHECK_TRUE(monitor.healthy(100U, 1000U));
    CHECK_TRUE(!monitor.healthy(1201U, 1000U));
    monitor.recordQueueHighWater(0U, 64U);
    CHECK_EQ(monitor.queueHighWater(0U), 64U);
    return EXIT_SUCCESS;
}
