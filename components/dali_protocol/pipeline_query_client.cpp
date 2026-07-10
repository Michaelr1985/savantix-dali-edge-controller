#include "dali/protocol/pipeline_query_client.h"

#include "dali/protocol/dali_frame.h"

namespace dali::protocol {
namespace {

DaliQueryOutcome outcomeFromResult(
    const DaliTransactionResult& result) noexcept {
    if (result.status == TransactionStatus::Ok && result.response.has_value()) {
        return DaliQueryOutcome::Ok;
    }
    switch (result.lastPhyResult) {
        case PhyResult::Timeout:
            return DaliQueryOutcome::NoResponse;
        case PhyResult::Collision:
            return DaliQueryOutcome::Collision;
        case PhyResult::Busy:
            return DaliQueryOutcome::Busy;
        case PhyResult::BusPowerLost:
            return DaliQueryOutcome::BusFailure;
        case PhyResult::TransceiverFault:
        case PhyResult::NotInitialised:
        case PhyResult::InternalError:
            return DaliQueryOutcome::HardwareFailure;
        case PhyResult::InvalidArgument:
        case PhyResult::Ok:
            return DaliQueryOutcome::Invalid;
    }
    return DaliQueryOutcome::Invalid;
}

Reading<std::uint8_t> readingForResult(
    const DaliTransactionResult& result,
    DaliQueryOutcome outcome) noexcept {
    if (outcome == DaliQueryOutcome::Ok && result.response.has_value()) {
        return Reading<std::uint8_t>::valid(*result.response, 0, 0);
    }
    if (outcome == DaliQueryOutcome::NoResponse ||
        outcome == DaliQueryOutcome::Collision ||
        outcome == DaliQueryOutcome::Busy) {
        return Reading<std::uint8_t>::temporarilyUnavailable();
    }
    return Reading<std::uint8_t>::invalid();
}

}  // namespace

DaliQueryResult PipelineQueryClient::query(
    ShortAddress address,
    DaliCommand command,
    CommandPriority priority) noexcept {
    const DaliRequest request{
        makeShortCommand(address, command),
        true,
        timeoutMs_,
        maximumRetries_,
        priority,
        address.value(),
        nextSequence_++,
    };
    if (pipeline_.enqueue(request) != QueueResult::Ok) {
        return {DaliQueryOutcome::QueueFull,
                Reading<std::uint8_t>::temporarilyUnavailable(), 0,
                PhyResult::Busy};
    }
    const auto processed = pipeline_.processNext();
    if (!processed.has_value()) {
        return {DaliQueryOutcome::Invalid, Reading<std::uint8_t>::invalid(), 0,
                PhyResult::InternalError};
    }
    const DaliQueryOutcome outcome = outcomeFromResult(processed->result);
    return {outcome,
            readingForResult(processed->result, outcome),
            processed->result.attempts,
            processed->result.lastPhyResult};
}

}  // namespace dali::protocol
