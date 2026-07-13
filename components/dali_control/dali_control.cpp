#include "dali/control/dali_control.h"

#include "dali/protocol/dali_frame.h"

namespace dali::control {

ControlResult DaliControl::submit(protocol::ForwardFrame frame, std::uint8_t deviceKey, protocol::CommandPriority priority) noexcept {
    const protocol::DaliRequest request{frame, false, timeoutMs_, maxRetries_, priority, deviceKey, nextSequence_++};
    if (pipeline_.enqueue(request) != protocol::QueueResult::Ok) return ControlResult::QueueFull;
    const auto processed = pipeline_.processNext();
    if (!processed.has_value()) return ControlResult::TransactionFailed;
    return processed->result.status == protocol::TransactionStatus::Ok ? ControlResult::Ok : ControlResult::TransactionFailed;
}

ControlResult DaliControl::sendShort(ShortAddress address, protocol::DaliCommand command, protocol::CommandPriority priority) noexcept {
    return submit(protocol::makeShortCommand(address, command), address.value(), priority);
}

ControlResult DaliControl::sendGroup(GroupAddress address, protocol::DaliCommand command, protocol::CommandPriority priority) noexcept {
    return submit(protocol::makeGroupCommand(address, command), static_cast<std::uint8_t>(0x80U | address.value()), priority);
}

ControlResult DaliControl::sendBroadcast(protocol::DaliCommand command, protocol::CommandPriority priority) noexcept {
    return submit(protocol::makeBroadcastCommand(command), 0xFFU, priority);
}

ControlResult DaliControl::setShortLevel(ShortAddress address, std::uint8_t level, protocol::CommandPriority priority) noexcept {
    const auto arc = protocol::ArcLevel::fromRaw(level);
    if (!arc.has_value()) return ControlResult::InvalidArgument;
    return submit(protocol::makeShortArc(address, *arc), address.value(), priority);
}

ControlResult DaliControl::setGroupLevel(GroupAddress address, std::uint8_t level, protocol::CommandPriority priority) noexcept {
    const auto arc = protocol::ArcLevel::fromRaw(level);
    if (!arc.has_value()) return ControlResult::InvalidArgument;
    return submit(protocol::makeGroupArc(address, *arc), static_cast<std::uint8_t>(0x80U | address.value()), priority);
}

ControlResult DaliControl::setDtr0(std::uint8_t value, protocol::CommandPriority priority) noexcept {
    return sendSpecial(protocol::DaliSpecialCommand::SetDtr0, value, priority);
}

ControlResult DaliControl::sendSpecial(protocol::DaliSpecialCommand command, std::uint8_t parameter, protocol::CommandPriority priority) noexcept {
    return submit(protocol::makeSpecialCommand(command, parameter), 0xFFU, priority);
}

}  // namespace dali::control
