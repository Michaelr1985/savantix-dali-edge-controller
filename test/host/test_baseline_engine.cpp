#include <cstdlib>

#include "dali/baseline/baseline_engine.h"
#include "test_support.h"

int main() {
    using namespace dali::baseline;
    BaselineStore store;
    BaselineObservation sample{};
    sample.stable = true;
    sample.healthy = true;
    sample.communicationAcceptable = true;
    sample.inputPower = dali::Reading<float>::valid(100.0F, 0, 0);
    sample.outputCurrent = dali::Reading<float>::valid(2.0F, 0, 0);
    for (int i = 0; i < 10; ++i) CHECK_TRUE(store.update(100U, sample));
    const auto band = store.bandFor(100U);
    CHECK_EQ(band, LevelBand::Band5);
    CHECK_TRUE(store.meanInputPower(band) > 99.0F);

    BaselineObservation abnormal = sample;
    abnormal.healthy = false;
    abnormal.inputPower = dali::Reading<float>::valid(999.0F, 0, 0);
    CHECK_TRUE(!store.update(100U, abnormal));
    CHECK_TRUE(store.meanInputPower(band) < 101.0F);
    return EXIT_SUCCESS;
}
