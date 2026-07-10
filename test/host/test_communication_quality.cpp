#include <cstdlib>

#include "dali/protocol/dali_transaction.h"
#include "test_support.h"

namespace {

dali::protocol::DaliTransactionResult result(
    dali::protocol::TransactionStatus status,
    dali::PhyResult phyResult) {
    return {status, std::nullopt, 1U, phyResult, 0U};
}

}  // namespace

int main() {
    using namespace dali;
    using namespace dali::protocol;

    CommunicationQualityTracker tracker;
    tracker.record(result(TransactionStatus::Ok, PhyResult::Ok));
    tracker.record(result(TransactionStatus::Ok, PhyResult::Ok));
    tracker.record(result(TransactionStatus::RetriesExhausted,
                          PhyResult::Timeout));
    tracker.record(result(TransactionStatus::RetriesExhausted,
                          PhyResult::Collision));
    tracker.record(result(TransactionStatus::RetriesExhausted,
                          PhyResult::Busy));
    tracker.record(result(TransactionStatus::TransceiverFault,
                          PhyResult::TransceiverFault));

    const CommunicationQualitySnapshot snapshot = tracker.snapshot();
    CHECK_TRUE(!snapshot.noData);
    CHECK_EQ(snapshot.counters.total, 6U);
    CHECK_EQ(snapshot.counters.successes, 2U);
    CHECK_EQ(snapshot.counters.timeouts, 1U);
    CHECK_EQ(snapshot.counters.collisions, 1U);
    CHECK_EQ(snapshot.counters.busy, 1U);
    CHECK_EQ(snapshot.counters.transceiverFaults, 1U);
    CHECK_EQ(snapshot.failures, 4U);
    CHECK_TRUE(snapshot.successRate > 0.333F);
    CHECK_TRUE(snapshot.successRate < 0.334F);

    const CommunicationQualitySnapshot empty =
        CommunicationQualityTracker{}.snapshot();
    CHECK_TRUE(empty.noData);
    CHECK_EQ(empty.successRate, 1.0F);
    return EXIT_SUCCESS;
}
