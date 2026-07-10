#pragma once

#include "dali/monitor/system_monitor.h"

namespace dali::monitor {

class SystemMonitorTask final {
public:
    explicit SystemMonitorTask(SystemMonitor& monitor) noexcept : monitor_{monitor} {}
    void runOnce(std::uint64_t nowMs) noexcept;

private:
    SystemMonitor& monitor_;
};

}  // namespace dali::monitor
