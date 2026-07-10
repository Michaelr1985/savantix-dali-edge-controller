#pragma once

#include <cstddef>
#include <optional>

#include "dali/protocol/dali_command_queue.h"
#include "dali/protocol/dali_transaction.h"

namespace dali::protocol {

struct ProcessedDaliRequest final {
    DaliRequest request{};
    DaliTransactionResult result{};
};

class DaliProtocolPipeline final {
public:
    explicit DaliProtocolPipeline(DaliTransactionService& transaction) noexcept
        : transaction_{transaction} {}

    [[nodiscard]] QueueResult enqueue(const DaliRequest& request) noexcept {
        return queue_.enqueue(request);
    }

    [[nodiscard]] std::optional<ProcessedDaliRequest> processNext() noexcept {
        const std::optional<DaliRequest> request = queue_.pop();
        if (!request.has_value()) {
            return std::nullopt;
        }
        DaliTransactionResult result = transaction_.execute(*request);
        quality_.record(result);
        return ProcessedDaliRequest{*request, result};
    }

    [[nodiscard]] CommunicationQualitySnapshot quality() const noexcept {
        return quality_.snapshot();
    }

    [[nodiscard]] std::size_t queued() const noexcept {
        return queue_.size();
    }

private:
    DaliCommandQueue queue_{};
    DaliTransactionService& transaction_;
    CommunicationQualityTracker quality_{};
};

}  // namespace dali::protocol
