#include "dali/protocol/dali_transaction.h"

#include <limits>

namespace dali::protocol {

void CommunicationQualityTracker::record(
    const DaliTransactionResult& result) noexcept {
    incrementSaturating(counters_.total);
    if (result.status == TransactionStatus::Ok) {
        incrementSaturating(counters_.successes);
        return;
    }

    switch (result.lastPhyResult) {
        case PhyResult::Timeout:
            incrementSaturating(counters_.timeouts);
            break;
        case PhyResult::Collision:
            incrementSaturating(counters_.collisions);
            break;
        case PhyResult::Busy:
            incrementSaturating(counters_.busy);
            break;
        case PhyResult::BusPowerLost:
            incrementSaturating(counters_.busPowerLosses);
            break;
        case PhyResult::TransceiverFault:
            incrementSaturating(counters_.transceiverFaults);
            break;
        case PhyResult::Ok:
        case PhyResult::InvalidArgument:
        case PhyResult::NotInitialised:
        case PhyResult::InternalError:
            incrementSaturating(counters_.otherFailures);
            break;
    }
}

CommunicationQualitySnapshot CommunicationQualityTracker::snapshot()
    const noexcept {
    const bool noData = counters_.total == 0U;
    const std::uint32_t failures =
        counters_.total >= counters_.successes
            ? counters_.total - counters_.successes
            : 0U;
    const float successRate = noData
        ? 1.0F
        : static_cast<float>(counters_.successes) /
              static_cast<float>(counters_.total);
    return {counters_, failures, successRate, noData};
}

void CommunicationQualityTracker::incrementSaturating(
    std::uint32_t& value) noexcept {
    if (value != std::numeric_limits<std::uint32_t>::max()) {
        ++value;
    }
}

DaliTransactionResult DaliTransactionService::execute(
    const DaliRequest& request) noexcept {
    DaliTransactionResult result{};
    if (request.timeoutMs == 0U ||
        request.maxRetries > kAbsoluteMaxRetries) {
        result.status = TransactionStatus::InvalidRequest;
        result.lastPhyResult = PhyResult::InvalidArgument;
        return result;
    }

    for (std::uint8_t attempt = 0;
         attempt <= request.maxRetries;
         ++attempt) {
        ++result.attempts;
        PhyResult phyResult = phy_.transmit(request.frame.raw);
        std::uint8_t backwardFrame = 0;
        if (phyResult == PhyResult::Ok && request.expectsResponse) {
            phyResult = phy_.receive(backwardFrame, request.timeoutMs);
        }
        result.lastPhyResult = phyResult;

        if (phyResult == PhyResult::Ok) {
            result.status = TransactionStatus::Ok;
            if (request.expectsResponse) {
                result.response = backwardFrame;
            }
            return result;
        }

        if (!isRetryable(phyResult)) {
            result.status = mapTerminalStatus(phyResult);
            return result;
        }

        if (attempt == request.maxRetries) {
            result.status = TransactionStatus::RetriesExhausted;
            return result;
        }

        const std::uint32_t retryDelay =
            baseRetryDelayMs(attempt) + jitter_.next(kMaximumJitterMs);
        result.retryDelayTotalMs += retryDelay;
        delay_.delayMs(retryDelay);
    }

    result.status = TransactionStatus::InternalError;
    result.lastPhyResult = PhyResult::InternalError;
    return result;
}

bool DaliTransactionService::isRetryable(PhyResult result) noexcept {
    return result == PhyResult::Timeout || result == PhyResult::Busy ||
           result == PhyResult::Collision;
}

std::uint32_t DaliTransactionService::baseRetryDelayMs(
    std::uint8_t retryIndex) noexcept {
    if (retryIndex == 0U) {
        return 100U;
    }
    if (retryIndex == 1U) {
        return 500U;
    }
    return 2000U;
}

TransactionStatus DaliTransactionService::mapTerminalStatus(
    PhyResult result) noexcept {
    switch (result) {
        case PhyResult::BusPowerLost:
            return TransactionStatus::BusPowerLost;
        case PhyResult::TransceiverFault:
            return TransactionStatus::TransceiverFault;
        case PhyResult::NotInitialised:
            return TransactionStatus::NotInitialised;
        case PhyResult::InvalidArgument:
            return TransactionStatus::InvalidRequest;
        case PhyResult::Timeout:
        case PhyResult::Busy:
        case PhyResult::Collision:
            return TransactionStatus::RetriesExhausted;
        case PhyResult::Ok:
            return TransactionStatus::Ok;
        case PhyResult::InternalError:
            return TransactionStatus::InternalError;
    }
    return TransactionStatus::InternalError;
}

}  // namespace dali::protocol
