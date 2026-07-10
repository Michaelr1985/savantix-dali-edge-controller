#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "dali/protocol/dali_transaction.h"
#include "dali/sim/simulated_dali_phy.h"
#include "test_support.h"

namespace {

class RecordingDelay final : public dali::protocol::IDelayProvider {
public:
    void delayMs(std::uint32_t delay) noexcept override {
        if (count_ < values_.size()) {
            values_[count_++] = delay;
        }
    }

    [[nodiscard]] std::size_t count() const noexcept { return count_; }
    [[nodiscard]] std::uint32_t at(std::size_t index) const noexcept {
        return values_[index];
    }

private:
    std::array<std::uint32_t, 8> values_{};
    std::size_t count_{0};
};

class FixedJitter final : public dali::protocol::IJitterSource {
public:
    std::uint32_t next(std::uint32_t maxInclusive) noexcept override {
        return value_ <= maxInclusive ? value_ : maxInclusive;
    }
    std::uint32_t value_{7};
};

dali::protocol::DaliRequest query(std::uint8_t retries = 3U) {
    using namespace dali::protocol;
    return {ForwardFrame{0x0790U}, true, 20U, retries,
            CommandPriority::Normal, 3U, 1U};
}

}  // namespace

int main() {
    using namespace dali;
    using namespace dali::protocol;

    {
        SimulatedDaliPhy phy;
        RecordingDelay delay;
        FixedJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        CHECK_EQ(phy.enqueueResponse({PhyResult::Ok, 0xA5U, 0U}), PhyResult::Ok);
        DaliTransactionService service{phy, delay, jitter};
        const DaliTransactionResult result = service.execute(query());
        CHECK_EQ(result.status, TransactionStatus::Ok);
        CHECK_EQ(result.attempts, 1U);
        CHECK_TRUE(result.response.has_value());
        CHECK_EQ(*result.response, 0xA5U);
        CHECK_EQ(delay.count(), 0U);
    }

    {
        SimulatedDaliPhy phy;
        RecordingDelay delay;
        FixedJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        DaliTransactionService service{phy, delay, jitter};
        DaliRequest command = query();
        command.expectsResponse = false;
        const DaliTransactionResult result = service.execute(command);
        CHECK_EQ(result.status, TransactionStatus::Ok);
        CHECK_EQ(result.attempts, 1U);
        CHECK_TRUE(!result.response.has_value());
    }

    {
        SimulatedDaliPhy phy;
        RecordingDelay delay;
        FixedJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        CHECK_EQ(phy.enqueueResponse({PhyResult::Timeout, 0U, 0U}), PhyResult::Ok);
        CHECK_EQ(phy.enqueueResponse({PhyResult::Timeout, 0U, 0U}), PhyResult::Ok);
        CHECK_EQ(phy.enqueueResponse({PhyResult::Ok, 0x55U, 0U}), PhyResult::Ok);
        DaliTransactionService service{phy, delay, jitter};
        const DaliTransactionResult result = service.execute(query());
        CHECK_EQ(result.status, TransactionStatus::Ok);
        CHECK_EQ(result.attempts, 3U);
        CHECK_EQ(delay.count(), 2U);
        CHECK_EQ(delay.at(0), 107U);
        CHECK_EQ(delay.at(1), 507U);
        CHECK_EQ(result.retryDelayTotalMs, 614U);
    }

    {
        SimulatedDaliPhy phy;
        RecordingDelay delay;
        FixedJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        for (int index = 0; index < 4; ++index) {
            CHECK_EQ(phy.enqueueResponse({PhyResult::Timeout, 0U, 0U}),
                     PhyResult::Ok);
        }
        DaliTransactionService service{phy, delay, jitter};
        const DaliTransactionResult result = service.execute(query(3U));
        CHECK_EQ(result.status, TransactionStatus::RetriesExhausted);
        CHECK_EQ(result.attempts, 4U);
        CHECK_EQ(delay.count(), 3U);
        CHECK_EQ(delay.at(2), 2007U);
    }

    for (const PhyResult retryable : {PhyResult::Collision, PhyResult::Busy}) {
        SimulatedDaliPhy phy;
        RecordingDelay delay;
        FixedJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        CHECK_EQ(phy.enqueueResponse({retryable, 0U, 0U}), PhyResult::Ok);
        CHECK_EQ(phy.enqueueResponse({PhyResult::Ok, 0x11U, 0U}), PhyResult::Ok);
        DaliTransactionService service{phy, delay, jitter};
        const DaliTransactionResult result = service.execute(query(1U));
        CHECK_EQ(result.status, TransactionStatus::Ok);
        CHECK_EQ(result.attempts, 2U);
    }

    {
        SimulatedDaliPhy phy;
        RecordingDelay delay;
        FixedJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        phy.setBusPowered(false);
        DaliTransactionService service{phy, delay, jitter};
        const DaliTransactionResult result = service.execute(query());
        CHECK_EQ(result.status, TransactionStatus::BusPowerLost);
        CHECK_EQ(result.attempts, 1U);
        CHECK_EQ(delay.count(), 0U);
    }

    {
        SimulatedDaliPhy phy;
        RecordingDelay delay;
        FixedJitter jitter;
        CHECK_EQ(phy.init(), PhyResult::Ok);
        phy.setTransceiverHealthy(false);
        DaliTransactionService service{phy, delay, jitter};
        const DaliTransactionResult result = service.execute(query());
        CHECK_EQ(result.status, TransactionStatus::TransceiverFault);
        CHECK_EQ(result.attempts, 1U);
    }

    {
        SimulatedDaliPhy phy;
        RecordingDelay delay;
        FixedJitter jitter;
        DaliTransactionService service{phy, delay, jitter};
        DaliRequest invalid = query();
        invalid.timeoutMs = 0;
        const DaliTransactionResult result = service.execute(invalid);
        CHECK_EQ(result.status, TransactionStatus::InvalidRequest);
        CHECK_EQ(result.attempts, 0U);
        CHECK_EQ(phy.transmittedFrameCount(), 0U);
    }

    return EXIT_SUCCESS;
}
