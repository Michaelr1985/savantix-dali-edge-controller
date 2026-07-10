#include "dali/monitor/system_monitor.h"

namespace dali::monitor {

void SystemMonitor::setExpectedTaskCount(std::size_t count) noexcept {
    expectedTasks_ = count > kMaximumTasks ? kMaximumTasks : count;
}

void SystemMonitor::heartbeat(std::size_t taskId,
                              std::uint64_t timestampMs) noexcept {
    if (taskId >= kMaximumTasks) return;
    heartbeats_[taskId] = timestampMs;
    seen_[taskId] = true;
}

bool SystemMonitor::healthy(std::uint64_t nowMs,
                            std::uint64_t timeoutMs) const noexcept {
    for (std::size_t task = 0; task < expectedTasks_; ++task) {
        if (!seen_[task] || nowMs < heartbeats_[task] ||
            nowMs - heartbeats_[task] > timeoutMs) return false;
    }
    return true;
}

void SystemMonitor::recordQueueHighWater(std::size_t queueId,
                                         std::size_t value) noexcept {
    if (queueId >= kMaximumQueues) return;
    if (value > queueHighWater_[queueId]) queueHighWater_[queueId] = value;
}

std::size_t SystemMonitor::queueHighWater(std::size_t queueId) const noexcept {
    return queueId < kMaximumQueues ? queueHighWater_[queueId] : 0U;
}

}  // namespace dali::monitor
