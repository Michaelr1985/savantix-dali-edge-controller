#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "dali/protocol/dali_frame.h"

namespace dali::protocol {

enum class CommandPriority : std::uint8_t {
    Critical = 0,
    High = 1,
    Normal = 2,
    Low = 3,
};

enum class QueueResult : std::uint8_t {
    Ok,
    Full,
};

struct DaliRequest final {
    ForwardFrame frame{};
    bool expectsResponse{true};
    std::uint32_t timeoutMs{20};
    std::uint8_t maxRetries{3};
    CommandPriority priority{CommandPriority::Normal};
    std::uint8_t deviceKey{0xFF};
    std::uint32_t requestSequence{0};
};

class DaliCommandQueue final {
public:
    static constexpr std::size_t kCapacity = 64;

    [[nodiscard]] QueueResult enqueue(const DaliRequest& request) noexcept {
        if (count_ == kCapacity) {
            return QueueResult::Full;
        }

        std::size_t insertion = 0;
        const auto requestedPriority =
            static_cast<std::uint8_t>(request.priority);
        while (insertion < count_ &&
               static_cast<std::uint8_t>(items_[insertion].priority) <=
                   requestedPriority) {
            ++insertion;
        }
        for (std::size_t index = count_; index > insertion; --index) {
            items_[index] = items_[index - 1U];
        }
        items_[insertion] = request;
        ++count_;
        return QueueResult::Ok;
    }

    [[nodiscard]] std::optional<DaliRequest> pop() noexcept {
        if (count_ == 0) {
            return std::nullopt;
        }
        const DaliRequest request = items_[0];
        for (std::size_t index = 1; index < count_; ++index) {
            items_[index - 1U] = items_[index];
        }
        --count_;
        return request;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return count_;
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return count_ == 0;
    }

private:
    std::array<DaliRequest, kCapacity> items_{};
    std::size_t count_{0};
};

}  // namespace dali::protocol
