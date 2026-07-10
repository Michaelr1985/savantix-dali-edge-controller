#include <cstdint>
#include <cstdlib>

#include "dali/core/types.h"
#include "dali/protocol/dali_commands.h"
#include "dali/protocol/dali_frame.h"
#include "dali/protocol/dali_protocol_pipeline.h"
#include "dali/protocol/dali_response_parser.h"
#include "dali/protocol/dali_transaction.h"
#include "dali/sim/simulated_dali_phy.h"
#include "test_support.h"

namespace {

class NoDelay final : public dali::protocol::IDelayProvider {
public:
    void delayMs(std::uint32_t) noexcept override {}
};

class ZeroJitter final : public dali::protocol::IJitterSource {
public:
    std::uint32_t next(std::uint32_t) noexcept override { return 0; }
};

}  // namespace

int main() {
    using namespace dali;
    using namespace dali::protocol;

    SimulatedDaliPhy phy;
    CHECK_EQ(phy.init(), PhyResult::Ok);
    CHECK_EQ(phy.enqueueResponse({PhyResult::Ok, 0x04U, 0U}), PhyResult::Ok);
    NoDelay delay;
    ZeroJitter jitter;
    DaliTransactionService service{phy, delay, jitter};
    DaliProtocolPipeline pipeline{service};

    const auto address = ShortAddress::fromRaw(3);
    CHECK_TRUE(address.has_value());
    const DaliRequest request{makeShortCommand(*address, DaliCommand::QueryStatus),
                              true, 20U, 3U, CommandPriority::Normal, 3U, 42U};
    CHECK_EQ(pipeline.enqueue(request), QueueResult::Ok);
    const auto processed = pipeline.processNext();
    CHECK_TRUE(processed.has_value());
    CHECK_EQ(processed->request.requestSequence, 42U);
    CHECK_EQ(processed->result.status, TransactionStatus::Ok);
    CHECK_TRUE(processed->result.response.has_value());

    const Reading<BasicStatus> status = parseBasicStatus(
        makeBackwardResponse(processed->result.lastPhyResult,
                             *processed->result.response));
    CHECK_TRUE(status.hasValue());
    CHECK_TRUE(status.value.lampPowerOn);
    CHECK_TRUE(!status.value.lampFailure);
    CHECK_EQ(pipeline.quality().counters.successes, 1U);
    CHECK_TRUE(!pipeline.processNext().has_value());
    return EXIT_SUCCESS;
}
