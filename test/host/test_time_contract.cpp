#include <cstdint>
#include <cstdlib>

#include "dali/time/i_time_source.h"
#include "test_support.h"

namespace {

class FakeTimeSource final : public dali::ITimeSource {
public:
    [[nodiscard]] std::uint64_t uptimeMs() const noexcept override {
        return uptimeMs_;
    }

    [[nodiscard]] bool unixTimeValid() const noexcept override {
        return synchronised_;
    }

    [[nodiscard]] std::uint64_t unixTimeSeconds() const noexcept override {
        if (!synchronised_) {
            return 0;
        }
        return unixAtSyncSeconds_ + (uptimeMs_ - uptimeAtSyncMs_) / 1000U;
    }

    void synchroniseUnixTime(std::uint64_t unixSeconds,
                             std::uint64_t uptimeAtSyncMs) noexcept override {
        unixAtSyncSeconds_ = unixSeconds;
        uptimeAtSyncMs_ = uptimeAtSyncMs;
        synchronised_ = true;
    }

    void setUptimeMs(std::uint64_t uptimeMs) noexcept {
        uptimeMs_ = uptimeMs;
    }

private:
    std::uint64_t uptimeMs_{0};
    std::uint64_t unixAtSyncSeconds_{0};
    std::uint64_t uptimeAtSyncMs_{0};
    bool synchronised_{false};
};

}  // namespace

int main() {
    FakeTimeSource time;
    time.setUptimeMs(5000U);
    CHECK_EQ(time.uptimeMs(), 5000U);
    CHECK_TRUE(!time.unixTimeValid());
    CHECK_EQ(time.unixTimeSeconds(), 0U);

    time.synchroniseUnixTime(1'800'000'000U, 5000U);
    time.setUptimeMs(8000U);
    CHECK_TRUE(time.unixTimeValid());
    CHECK_EQ(time.unixTimeSeconds(), 1'800'000'003U);
    return EXIT_SUCCESS;
}
