#include <cstdint>
#include <cstdlib>
#include <utility>

#include "dali/core/types.h"
#include "dali/protocol/dali_commands.h"
#include "dali/protocol/pipeline_query_client.h"
#include "dali/protocol/dali_protocol_pipeline.h"
#include "dali/protocol/dali_transaction.h"
#include "dali/sim/simulated_dali_phy.h"
#include "test_support.h"

namespace {

class NoDelay final : public dali::protocol::IDelayProvider {
public:
    void delayMs(std::uint32_t) noexcept override {}
};

class NoJitter final : public dali::protocol::IJitterSource {
public:
    std::uint32_t next(std::uint32_t) noexcept override { return 0; }
};

}  // namespace

int main() {
    using namespace dali;
    using namespace dali::protocol;
    const ShortAddress address = *ShortAddress::fromRaw(3U);

    {
        SimulatedDaliPhy phy;
        NoDelay delay;
        NoJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        CHECK_EQ(phy.enqueueResponse({PhyResult::Ok, 0x00U, 0U}), PhyResult::Ok);
        DaliTransactionService transaction{phy, delay, jitter};
        DaliProtocolPipeline pipeline{transaction};
        PipelineQueryClient client{pipeline, 20U, 3U};
        const DaliQueryResult result = client.query(
            address, DaliCommand::QueryStatus, CommandPriority::High);
        CHECK_EQ(result.outcome, DaliQueryOutcome::Ok);
        CHECK_TRUE(result.response.hasValue());
        CHECK_EQ(result.response.value, 0U);
        CHECK_EQ(result.attempts, 1U);
    }

    for (const auto& scenario : {
             std::pair{PhyResult::Timeout, DaliQueryOutcome::NoResponse},
             std::pair{PhyResult::Collision, DaliQueryOutcome::Collision}}) {
        SimulatedDaliPhy phy;
        NoDelay delay;
        NoJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        for (int attempt = 0; attempt < 4; ++attempt) {
            CHECK_EQ(phy.enqueueResponse({scenario.first, 0U, 0U}),
                     PhyResult::Ok);
        }
        DaliTransactionService transaction{phy, delay, jitter};
        DaliProtocolPipeline pipeline{transaction};
        PipelineQueryClient client{pipeline, 20U, 3U};
        const DaliQueryResult result = client.query(
            address, DaliCommand::QueryStatus, CommandPriority::High);
        CHECK_EQ(result.outcome, scenario.second);
        CHECK_TRUE(!result.response.hasValue());
        CHECK_EQ(result.attempts, 4U);
    }

    {
        SimulatedDaliPhy phy;
        NoDelay delay;
        NoJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        phy.setBusPowered(false);
        DaliTransactionService transaction{phy, delay, jitter};
        DaliProtocolPipeline pipeline{transaction};
        PipelineQueryClient client{pipeline, 20U, 3U};
        const DaliQueryResult result = client.query(
            address, DaliCommand::QueryStatus, CommandPriority::High);
        CHECK_EQ(result.outcome, DaliQueryOutcome::BusFailure);
        CHECK_EQ(result.attempts, 1U);
    }
    return EXIT_SUCCESS;
}
