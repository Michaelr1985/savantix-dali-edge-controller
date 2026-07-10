#pragma once

#include <cstdint>

#include "dali/core/types.h"
#include "dali/phy/i_dali_phy.h"
#include "dali/protocol/dali_command_queue.h"
#include "dali/protocol/dali_commands.h"

namespace dali::protocol {

enum class DaliQueryOutcome : std::uint8_t {
    Ok,
    NoResponse,
    Collision,
    Busy,
    BusFailure,
    HardwareFailure,
    QueueFull,
    Invalid,
};

struct DaliQueryResult final {
    DaliQueryOutcome outcome{DaliQueryOutcome::Invalid};
    Reading<std::uint8_t> response{};
    std::uint8_t attempts{0};
    PhyResult phyResult{PhyResult::InternalError};
};

class IDaliQueryClient {
public:
    virtual ~IDaliQueryClient() = default;
    [[nodiscard]] virtual DaliQueryResult query(
        ShortAddress address,
        DaliCommand command,
        CommandPriority priority) noexcept = 0;
};

}  // namespace dali::protocol
