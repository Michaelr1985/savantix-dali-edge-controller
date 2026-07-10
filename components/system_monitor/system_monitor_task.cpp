#include "dali/monitor/system_monitor_task.h"

namespace dali::monitor {

void SystemMonitorTask::runOnce(std::uint64_t nowMs) noexcept {
    monitor_.heartbeat(0U, nowMs);
}

}  // namespace dali::monitor
