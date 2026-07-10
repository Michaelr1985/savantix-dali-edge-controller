#pragma once

#include <cstdint>
#include <optional>

#include "dali/phy/i_dali_phy.h"
#include "dali/protocol/dali_command_queue.h"

namespace dali::protocol {

class IDelayProvider {
public:
    virtual ~IDelayProvider() = default;
    virtual void delayMs(std::uint32_t delayMs) noexcept = 0;
};

class IJitterSource {
public:
    virtual ~IJitterSource() = default;
    virtual std::uint32_t next(std::uint32_t maxInclusive) noexcept = 0;
};

enum class TransactionStatus : std::uint8_t {
    Ok,
    RetriesExhausted,
    InvalidRequest,
    BusPowerLost,
    TransceiverFault,
    NotInitialised,
    InternalError,
};

struct DaliTransactionResult final {
    TransactionStatus status{TransactionStatus::InternalError};
    std::optional<std::uint8_t> response{};
    std::uint8_t attempts{0};
    PhyResult lastPhyResult{PhyResult::InternalError};
    std::uint32_t retryDelayTotalMs{0};
};

class DaliTransactionService final {
public:
    static constexpr std::uint8_t kAbsoluteMaxRetries = 8;
    static constexpr std::uint32_t kMaximumJitterMs = 25;

    DaliTransactionService(IDaliPhy& phy,
                           IDelayProvider& delay,
                           IJitterSource& jitter) noexcept
        : phy_{phy}, delay_{delay}, jitter_{jitter} {}

    [[nodiscard]] DaliTransactionResult execute(
        const DaliRequest& request) noexcept;

private:
    [[nodiscard]] static bool isRetryable(PhyResult result) noexcept;
    [[nodiscard]] static std::uint32_t baseRetryDelayMs(
        std::uint8_t retryIndex) noexcept;
    [[nodiscard]] static TransactionStatus mapTerminalStatus(
        PhyResult result) noexcept;

    IDaliPhy& phy_;
    IDelayProvider& delay_;
    IJitterSource& jitter_;
};

}  // namespace dali::protocol
