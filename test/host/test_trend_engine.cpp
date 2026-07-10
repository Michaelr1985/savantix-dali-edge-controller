#include <cstdlib>

#include "dali/trend/trend_engine.h"
#include "test_support.h"

int main() {
    using namespace dali::trend;
    RollingBaseline baseline;
    for (std::uint32_t i = 0; i < 5U; ++i) baseline.add(10.0F + i);
    CHECK_EQ(baseline.sampleCount(), 5U);
    CHECK_TRUE(baseline.mean() > 11.9F && baseline.mean() < 12.1F);
    CHECK_TRUE(baseline.standardDeviation() > 1.4F);

    TrendSeries series;
    for (std::uint64_t hour = 0; hour < 24U * 8U; ++hour) {
        series.add(hour * 3600000ULL, 100.0F - static_cast<float>(hour));
    }
    const TrendResult trend = series.evaluate(24U * 8U * 3600000ULL);
    CHECK_TRUE(trend.sampleCount >= 24U * 7U);
    CHECK_TRUE(trend.sevenDayAverage < trend.thirtyDayAverage ||
               trend.thirtyDayAverage == 0.0F);
    CHECK_TRUE(trend.rateOfChange < 0.0F);
    return EXIT_SUCCESS;
}
