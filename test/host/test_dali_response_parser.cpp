#include <cstdlib>

#include "dali/protocol/dali_response_parser.h"
#include "test_support.h"

int main() {
    using namespace dali;
    using namespace dali::protocol;

    const BackwardResponse yes = makeBackwardResponse(PhyResult::Ok, 0xFFU);
    const Reading<bool> yesValue = parseYesNo(yes);
    CHECK_TRUE(yesValue.hasValue());
    CHECK_TRUE(yesValue.value);

    const BackwardResponse no = makeBackwardResponse(PhyResult::Ok, 0x00U);
    const Reading<bool> noValue = parseYesNo(no);
    CHECK_TRUE(noValue.hasValue());
    CHECK_TRUE(!noValue.value);

    const Reading<BasicStatus> status = parseBasicStatus(
        makeBackwardResponse(PhyResult::Ok, 0xFFU));
    CHECK_TRUE(status.hasValue());
    CHECK_TRUE(status.value.controlGearFailure);
    CHECK_TRUE(status.value.lampFailure);
    CHECK_TRUE(status.value.lampPowerOn);
    CHECK_TRUE(status.value.limitError);
    CHECK_TRUE(status.value.fadeRunning);
    CHECK_TRUE(status.value.resetState);
    CHECK_TRUE(status.value.missingShortAddress);
    CHECK_TRUE(status.value.powerFailure);

    for (const PhyResult result : {PhyResult::Timeout, PhyResult::Busy,
                                   PhyResult::Collision}) {
        const BackwardResponse unavailable = makeBackwardResponse(result, 0xFFU);
        CHECK_EQ(unavailable.quality, ReadingQuality::TemporarilyUnavailable);
        CHECK_TRUE(!parseYesNo(unavailable).hasValue());
    }

    const BackwardResponse hardwareFault =
        makeBackwardResponse(PhyResult::TransceiverFault, 0xFFU);
    CHECK_EQ(hardwareFault.quality, ReadingQuality::Invalid);
    CHECK_TRUE(!parseBasicStatus(hardwareFault).hasValue());
    return EXIT_SUCCESS;
}
