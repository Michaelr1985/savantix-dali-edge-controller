#pragma once

#include <cstdint>

#include "dali/time/i_time_source.h"
#include "freertos/FreeRTOS.h"

namespace dali {

class EspTimeSource final : public ITimeSource {
public:
    [[nodiscard]] std::uint64_t uptimeMs() const noexcept override;
    [[nodiscard]] bool unixTimeValid() const noexcept override;
    [[nodiscard]] std::uint64_t unixTimeSeconds() const noexcept override;
    void synchroniseUnixTime(std::uint64_t unixSeconds,
                             std::uint64_t uptimeAtSyncMs) noexcept override;

private:
    mutable portMUX_TYPE lock_ = portMUX_INITIALIZER_UNLOCKED;
    std::uint64_t unixAtSyncSeconds_{0};
    std::uint64_t uptimeAtSyncMs_{0};
    bool synchronised_{false};
};

}  // namespace dali
