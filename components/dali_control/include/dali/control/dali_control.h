#pragma once

#include <cstdint>

#include "dali/core/types.h"
#include "dali/protocol/dali_protocol_pipeline.h"

namespace dali::control {

enum class ControlResult : std::uint8_t { Ok, QueueFull, TransactionFailed, InvalidArgument };

class DaliControl final {
public:
    DaliControl(protocol::DaliProtocolPipeline& pipeline,
                std::uint32_t timeoutMs = 20U,
                std::uint8_t maxRetries = 3U) noexcept
        : pipeline_{pipeline}, timeoutMs_{timeoutMs}, maxRetries_{maxRetries} {}

    [[nodiscard]] ControlResult sendShort(ShortAddress address, protocol::DaliCommand command, protocol::CommandPriority priority = protocol::CommandPriority::Normal) noexcept;
    [[nodiscard]] ControlResult sendGroup(GroupAddress address, protocol::DaliCommand command, protocol::CommandPriority priority = protocol::CommandPriority::Normal) noexcept;
    [[nodiscard]] ControlResult sendBroadcast(protocol::DaliCommand command, protocol::CommandPriority priority = protocol::CommandPriority::Normal) noexcept;
    [[nodiscard]] ControlResult setShortLevel(ShortAddress address, std::uint8_t level, protocol::CommandPriority priority = protocol::CommandPriority::Normal) noexcept;
    [[nodiscard]] ControlResult setGroupLevel(GroupAddress address, std::uint8_t level, protocol::CommandPriority priority = protocol::CommandPriority::Normal) noexcept;
    [[nodiscard]] ControlResult setDtr0(std::uint8_t value, protocol::CommandPriority priority = protocol::CommandPriority::High) noexcept;
    [[nodiscard]] ControlResult sendSpecial(protocol::DaliSpecialCommand command, std::uint8_t parameter, protocol::CommandPriority priority = protocol::CommandPriority::High) noexcept;

private:
    [[nodiscard]] ControlResult submit(protocol::ForwardFrame frame, std::uint8_t deviceKey, protocol::CommandPriority priority) noexcept;
    protocol::DaliProtocolPipeline& pipeline_;
    std::uint32_t timeoutMs_{20};
    std::uint8_t maxRetries_{3};
    std::uint32_t nextSequence_{1};
};

}  // namespace dali::control
