#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "dali/protocol/dali_command_queue.h"
#include "test_support.h"

namespace {

dali::protocol::DaliRequest request(
    std::uint32_t sequence,
    dali::protocol::CommandPriority priority) {
    using namespace dali::protocol;
    return {ForwardFrame{static_cast<std::uint16_t>(sequence)}, true, 20U, 3U,
            priority, 0xFFU, sequence};
}

}  // namespace

int main() {
    using namespace dali::protocol;
    DaliCommandQueue queue;

    CHECK_EQ(queue.enqueue(request(1U, CommandPriority::Normal)), QueueResult::Ok);
    CHECK_EQ(queue.enqueue(request(2U, CommandPriority::Normal)), QueueResult::Ok);
    CHECK_EQ(queue.enqueue(request(3U, CommandPriority::Low)), QueueResult::Ok);
    CHECK_EQ(queue.enqueue(request(4U, CommandPriority::Critical)), QueueResult::Ok);
    CHECK_EQ(queue.size(), 4U);

    CHECK_EQ(queue.pop()->requestSequence, 4U);
    CHECK_EQ(queue.pop()->requestSequence, 1U);
    CHECK_EQ(queue.pop()->requestSequence, 2U);
    CHECK_EQ(queue.pop()->requestSequence, 3U);
    CHECK_TRUE(!queue.pop().has_value());

    for (std::size_t index = 0; index < DaliCommandQueue::kCapacity; ++index) {
        CHECK_EQ(queue.enqueue(request(static_cast<std::uint32_t>(index),
                                       CommandPriority::High)),
                 QueueResult::Ok);
    }
    CHECK_EQ(queue.enqueue(request(1000U, CommandPriority::Critical)),
             QueueResult::Full);
    CHECK_EQ(queue.size(), DaliCommandQueue::kCapacity);
    return EXIT_SUCCESS;
}
