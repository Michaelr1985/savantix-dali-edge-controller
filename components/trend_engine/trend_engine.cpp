#include "dali/trend/trend_engine.h"

#include <cmath>

namespace dali::trend {

void RollingBaseline::add(float value) noexcept {
    if (count_ == 0U) {
        minimum_ = maximum_ = value;
    } else {
        if (value < minimum_) minimum_ = value;
        if (value > maximum_) maximum_ = value;
    }
    ++count_;
    const float delta = value - mean_;
    mean_ += delta / static_cast<float>(count_);
    const float delta2 = value - mean_;
    m2_ += delta * delta2;
}

float RollingBaseline::mean() const noexcept { return mean_; }

float RollingBaseline::standardDeviation() const noexcept {
    return count_ < 2U ? 0.0F
                       : std::sqrt(m2_ / static_cast<float>(count_ - 1U));
}

void TrendSeries::add(std::uint64_t timestampMs, float value) noexcept {
    samples_[next_] = {timestampMs, value};
    next_ = (next_ + 1U) % kCapacity;
    if (count_ < kCapacity) ++count_;
}

TrendResult TrendSeries::evaluate(std::uint64_t nowMs) const noexcept {
    constexpr std::uint64_t sevenDaysMs = 7ULL * 24ULL * 3600000ULL;
    constexpr std::uint64_t thirtyDaysMs = 30ULL * 24ULL * 3600000ULL;
    float sevenSum = 0.0F;
    float thirtySum = 0.0F;
    float ewma = 0.0F;
    float oldest = 0.0F;
    float newest = 0.0F;
    std::uint32_t sevenCount = 0;
    std::uint32_t thirtyCount = 0;
    std::uint64_t oldestTime = nowMs;
    std::uint64_t newestTime = 0;
    for (std::size_t i = 0; i < count_; ++i) {
        const Sample& sample = samples_[i];
        if (sample.timestampMs > nowMs) continue;
        const std::uint64_t age = nowMs - sample.timestampMs;
        if (age <= thirtyDaysMs) {
            thirtySum += sample.value;
            ++thirtyCount;
        }
        if (age <= sevenDaysMs) {
            sevenSum += sample.value;
            ++sevenCount;
        }
        if (sample.timestampMs < oldestTime) { oldestTime = sample.timestampMs; oldest = sample.value; }
        if (sample.timestampMs >= newestTime) { newestTime = sample.timestampMs; newest = sample.value; }
        ewma = ewma == 0.0F ? sample.value : 0.2F * sample.value + 0.8F * ewma;
    }
    const float rate = newestTime > oldestTime
        ? (newest - oldest) / static_cast<float>(newestTime - oldestTime)
        : 0.0F;
    const float sevenAverage = sevenCount == 0U ? 0.0F : sevenSum / sevenCount;
    const float thirtyAverage = thirtyCount == 0U ? 0.0F : thirtySum / thirtyCount;
    return {thirtyCount, sevenAverage, thirtyAverage, ewma, rate,
            thirtyCount >= 10U && std::fabs(rate) > 0.000001F};
}

}  // namespace dali::trend
