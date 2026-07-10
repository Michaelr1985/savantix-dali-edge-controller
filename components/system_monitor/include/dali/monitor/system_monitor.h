#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dali::monitor {

class SystemMonitor final {
public:
    static constexpr std::size_t kMaximumTasks = 16;
    static constexpr std::size_t kMaximumQueues = 16;

    void setExpectedTaskCount(std::size_t count) noexcept;
    void heartbeat(std::size_t taskId, std::uint64_t timestampMs) noexcept;
    [[nodiscard]] bool healthy(std::uint64_t nowMs,
                               std::uint64_t timeoutMs) const noexcept;
    void recordQueueHighWater(std::size_t queueId, std::size_t value) noexcept;
    [[nodiscard]] std::size_t queueHighWater(std::size_t queueId) const noexcept;

private:
    std::array<std::uint64_t, kMaximumTasks> heartbeats_{};
    std::array<bool, kMaximumTasks> seen_{};
    std::array<std::size_t, kMaximumQueues> queueHighWater_{};
    std::size_t expectedTasks_{0};
};

}  // namespace dali::monitor
