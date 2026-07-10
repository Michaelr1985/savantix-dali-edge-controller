#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dali::trend {

class RollingBaseline final {
public:
    void add(float value) noexcept;
    [[nodiscard]] std::uint32_t sampleCount() const noexcept { return count_; }
    [[nodiscard]] float mean() const noexcept;
    [[nodiscard]] float standardDeviation() const noexcept;
    [[nodiscard]] float minimum() const noexcept { return count_ == 0 ? 0.0F : minimum_; }
    [[nodiscard]] float maximum() const noexcept { return count_ == 0 ? 0.0F : maximum_; }

private:
    std::uint32_t count_{0};
    float mean_{0.0F};
    float m2_{0.0F};
    float minimum_{0.0F};
    float maximum_{0.0F};
};

struct TrendResult final {
    std::uint32_t sampleCount{0};
    float sevenDayAverage{0.0F};
    float thirtyDayAverage{0.0F};
    float exponentiallyWeightedAverage{0.0F};
    float rateOfChange{0.0F};
    bool statisticallyMeaningful{false};
};

class TrendSeries final {
public:
    static constexpr std::size_t kCapacity = 1024;
    void add(std::uint64_t timestampMs, float value) noexcept;
    [[nodiscard]] TrendResult evaluate(std::uint64_t nowMs) const noexcept;

private:
    struct Sample final { std::uint64_t timestampMs{0}; float value{0.0F}; };
    std::array<Sample, kCapacity> samples_{};
    std::size_t next_{0};
    std::size_t count_{0};
};

}  // namespace dali::trend
