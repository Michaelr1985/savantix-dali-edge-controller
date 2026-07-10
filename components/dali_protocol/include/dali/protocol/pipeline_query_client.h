#pragma once

#include <cstdint>

#include "dali/protocol/dali_protocol_pipeline.h"
#include "dali/protocol/i_dali_query_client.h"

namespace dali::protocol {

class PipelineQueryClient final : public IDaliQueryClient {
public:
    PipelineQueryClient(DaliProtocolPipeline& pipeline,
                        std::uint32_t timeoutMs,
                        std::uint8_t maximumRetries) noexcept
        : pipeline_{pipeline},
          timeoutMs_{timeoutMs},
          maximumRetries_{maximumRetries} {}

    [[nodiscard]] DaliQueryResult query(
        ShortAddress address,
        DaliCommand command,
        CommandPriority priority) noexcept override;

private:
    DaliProtocolPipeline& pipeline_;
    std::uint32_t timeoutMs_{20};
    std::uint8_t maximumRetries_{3};
    std::uint32_t nextSequence_{1};
};

}  // namespace dali::protocol
