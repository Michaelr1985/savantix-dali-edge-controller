#pragma once

#include <cstdint>

namespace dali {

class ITimeSource {
public:
    virtual ~ITimeSource() = default;

    [[nodiscard]] virtual std::uint64_t uptimeMs() const noexcept = 0;
    [[nodiscard]] virtual bool unixTimeValid() const noexcept = 0;
    [[nodiscard]] virtual std::uint64_t unixTimeSeconds() const noexcept = 0;
    virtual void synchroniseUnixTime(std::uint64_t unixSeconds,
                                     std::uint64_t uptimeAtSyncMs) noexcept = 0;
};

}  // namespace dali
