#include "dali/time/esp_time_source.h"

#include "esp_timer.h"

namespace dali {

std::uint64_t EspTimeSource::uptimeMs() const noexcept {
    return static_cast<std::uint64_t>(esp_timer_get_time()) / 1000U;
}

bool EspTimeSource::unixTimeValid() const noexcept {
    portENTER_CRITICAL(&lock_);
    const bool valid = synchronised_;
    portEXIT_CRITICAL(&lock_);
    return valid;
}

std::uint64_t EspTimeSource::unixTimeSeconds() const noexcept {
    std::uint64_t unixAtSync = 0;
    std::uint64_t uptimeAtSync = 0;
    bool valid = false;
    portENTER_CRITICAL(&lock_);
    unixAtSync = unixAtSyncSeconds_;
    uptimeAtSync = uptimeAtSyncMs_;
    valid = synchronised_;
    portEXIT_CRITICAL(&lock_);

    if (!valid) {
        return 0;
    }
    const std::uint64_t currentUptime = uptimeMs();
    if (currentUptime < uptimeAtSync) {
        return unixAtSync;
    }
    return unixAtSync + (currentUptime - uptimeAtSync) / 1000U;
}

void EspTimeSource::synchroniseUnixTime(
    std::uint64_t unixSeconds,
    std::uint64_t uptimeAtSyncMs) noexcept {
    portENTER_CRITICAL(&lock_);
    unixAtSyncSeconds_ = unixSeconds;
    uptimeAtSyncMs_ = uptimeAtSyncMs;
    synchronised_ = true;
    portEXIT_CRITICAL(&lock_);
}

}  // namespace dali
